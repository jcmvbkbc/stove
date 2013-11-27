/*
 * Restricted set of one wire interface commands.
 * Assuming there's only one DS18B20 powered by an external supply.
 */

#include "owi.h"

#include <avr/io.h>
#include <util/delay.h>

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
unsigned char owi_init()
{
	//Master Tx reset pulse, 0 for 480mks minimum
	OWI_DDR  |= _BV(OWI_BIT);
	OWI_PORT &= ~_BV(OWI_BIT);
	_delay_us(485);
	OWI_DDR  &= ~_BV(OWI_BIT);

	//Master Rx 480 mks minimum
	//ds18b20 waits 15–60mks and then transmits a presence pulse, 0 for 60–240mks
	_delay_us(65);
	unsigned char res = !(OWI_PIN & _BV(OWI_BIT));
	_delay_us(420);

	return res;
}

void owi_write(unsigned char b)
{
	int i;
	for (i = 0; i < 8; ++i) {
		//minimum of a 1mks recovery time between individual write slots.
		OWI_DDR  &= ~_BV(OWI_BIT);
		_delay_us(2);

		//both types of write time slots are initiated by the master pulling the 1-wire bus low
		OWI_DDR  |= _BV(OWI_BIT);
		OWI_PORT &= ~_BV(OWI_BIT);
		_delay_us(2);

		if (b & _BV(i)) {
			//1: release the 1-wire bus within 15mks
			OWI_DDR &= ~_BV(OWI_BIT);
		} else {
			//0: hold the bus low for the duration of the time slot
		}

		_delay_us(65);
	}
}

unsigned char owi_read()
{
	unsigned char res = 0x00;

	int i;
	for (i = 0; i < 8; ++i) {
		//a minimum of a 1mks recovery time between slots
		OWI_DDR &= ~_BV(OWI_BIT);
		_delay_us(2);

		//read time slot is initiated by the master device pulling the 1-wire bus low
		//for a minimum of 1mks and then releasing the bus
		OWI_DDR  |= _BV(OWI_BIT);
		OWI_PORT &= ~_BV(OWI_BIT);
		_delay_us(2);
		OWI_DDR &= ~_BV(OWI_BIT);

		//Output data from the DS18B20 is valid for 15mks after the falling edge
		//that initiated the read time slot.
		//system timing margin is maximized by keeping T_INIT and T_RC as short as possible
		//and by locating the master sample time during read time slots towards the end of the 15mks period
		_delay_us(10);

		if (OWI_PIN & _BV(OWI_BIT)) {
			res |= _BV(i);
		}

		//All read time slots must be a minimum of 60mks in duration
		_delay_us(50);
	}

	return res;
}

void eeprom_write(unsigned addr, unsigned char data)
{
	while (EECR & _BV(EEPE));
	EEAR = addr;
	EEDR = data;
	EECR |= _BV(EEMPE);
	EECR |= _BV(EEPE);
}

float get_t()
{
	if (!owi_init()) {
		return T_UNDEF;
	}

	//address all devices on the bus simultaneously without sending out any ROM code information
	owi_write(SKIP_ROM);
	owi_write(CONVERT_T);

	//T_CONV = 750 mks for 12-bit resolution
	//If the DS18B20 is powered by an external supply,
	//the master can issue read time slots after the Convert T command
	//and the DS18B20 will respond by transmitting 0 while the temperature conversion is in progress
	//and 1 when the conversion is done.
	OWI_DDR &= ~_BV(OWI_BIT);
	while (!(OWI_PIN & _BV(OWI_BIT))) {
		_delay_us(50);
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

	eeprom_write(0, t0);
	eeprom_write(1, t1);
	eeprom_write(2, 0xa5);

	//T in 0..10, S in 11..15. S = 0 for positive numbers
	short res = (t1 << 8) | t0;

	//The resolution of the temperature sensor is user-configurable to 9, 10, 11, or 12 bits,
	//corresponding to increments of 0.5^C, 0.25^C, 0.125^C, and 0.0625^C, respectively.
	//The default resolution at power-up is 12 bit.
	return res * 0.0625;
}
