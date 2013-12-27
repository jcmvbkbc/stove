/*
 * Restricted set of one wire interface commands.
 * Assuming there's only one DS18B20 powered by an external supply.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "interrupt.h"
#include "owi.h"

#define OWI_DDR  DDRB
#define OWI_PORT PORTB
#define OWI_PIN  PINB
#define OWI_BIT  PB0

// OWI step 2: ROM command
#define SKIP_ROM 0xCC

//OWI step 3: function command
#define CONVERT_T 0x44
#define READ_SCRATCHPAD 0xBE

/*
 * OWI step 1
 */
uint8_t owi_init(void)
{
	uint8_t flags;
	uint8_t res;

	//Master Tx reset pulse, 0 for 480mks minimum
	OWI_PORT &= ~_BV(OWI_BIT);
	OWI_DDR  |= _BV(OWI_BIT);
	_delay_us(485);
	OWI_DDR  &= ~_BV(OWI_BIT);

	flags = cli_save_irq();
	//Master Rx 480 mks minimum
	//ds18b20 waits 15–60mks and then transmits a presence pulse, 0 for 60–240mks
	_delay_us(65);
	res = ~OWI_PIN;
	_delay_us(420);
	res &= OWI_PIN;
	restore_irq(flags);

	return (res & _BV(OWI_BIT)) != 0;
}

void owi_write(uint8_t b)
{
	uint8_t i;

	for (i = 0; i < 8; ++i, b >>= 1) {
		uint8_t flags;

		//minimum of a 1mks recovery time between individual write slots.
		OWI_DDR  &= ~_BV(OWI_BIT);
		_delay_us(2);

		//both types of write time slots are initiated by the master pulling the 1-wire bus low
		flags = cli_save_irq();
		OWI_PORT &= ~_BV(OWI_BIT);
		OWI_DDR  |= _BV(OWI_BIT);
		_delay_us(2);

		if (b & 1) {
			//1: release the 1-wire bus within 15mks
			OWI_DDR &= ~_BV(OWI_BIT);
		} else {
			//0: hold the bus low for the duration of the time slot
		}

		restore_irq(flags);
		_delay_us(65);
		OWI_DDR &= ~_BV(OWI_BIT);
	}
}

static uint8_t owi_read_bit(void)
{
	uint8_t res = 0;
	uint8_t flags;

	//a minimum of a 1mks recovery time between slots
	OWI_DDR &= ~_BV(OWI_BIT);
	_delay_us(2);

	//read time slot is initiated by the master device pulling the 1-wire bus low
	//for a minimum of 1mks and then releasing the bus
	flags = cli_save_irq();
	OWI_PORT &= ~_BV(OWI_BIT);
	OWI_DDR  |= _BV(OWI_BIT);
	_delay_us(2);
	OWI_DDR &= ~_BV(OWI_BIT);

	//Output data from the DS18B20 is valid for 15mks after the falling edge
	//that initiated the read time slot.
	//system timing margin is maximized by keeping T_INIT and T_RC as short as possible
	//and by locating the master sample time during read time slots towards the end of the 15mks period
	_delay_us(15);

	if (OWI_PIN & _BV(OWI_BIT))
		res = 1;

	restore_irq(flags);
	//All read time slots must be a minimum of 60mks in duration
	_delay_us(50);

	return res;
}

uint8_t owi_read(void)
{
	uint8_t res = 0x00;
	uint8_t i;

	for (i = 0; i < 8; ++i) {
		if (owi_read_bit())
			res |= _BV(i);
	}

	return res;
}

uint8_t start_get_t(void)
{
	if (!owi_init()) {
		return 0;
	}

	//address all devices on the bus simultaneously without sending out any ROM code information
	owi_write(SKIP_ROM);
	owi_write(CONVERT_T);
	return 1;
}

int read_t(void)
{
	//T_CONV = 750 ms for 12-bit resolution
	//If the DS18B20 is powered by an external supply,
	//the master can issue read time slots after the Convert T command
	//and the DS18B20 will respond by transmitting 0 while the temperature conversion is in progress
	//and 1 when the conversion is done.

	if (owi_read_bit() == 0) {
		return T_UNDEF;
	}

	if (!owi_init()) {
		return T_UNDEF;
	}

	owi_write(SKIP_ROM);
	owi_write(READ_SCRATCHPAD);

	unsigned char t0, t1;
	t0 = owi_read();
	t1 = owi_read();

	//The master may issue a reset to terminate reading at any time
	//if only part of the scratchpad data is needed
	owi_init();

	//T in 0..10, S in 11..15. S = 0 for positive numbers
	short res = (t1 << 8) | t0;

	//The resolution of the temperature sensor is user-configurable to 9, 10, 11, or 12 bits,
	//corresponding to increments of 0.5^C, 0.25^C, 0.125^C, and 0.0625^C, respectively.
	//The default resolution at power-up is 12 bit.
	return res;
}

int get_t(void)
{
	if (!start_get_t())
		return T_UNDEF;
	_delay_ms(750);
	return read_t();
}
