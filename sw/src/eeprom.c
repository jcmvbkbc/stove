#include <avr/io.h>
#include "eeprom.h"
#include "interrupt.h"

void eeprom_write(unsigned addr, uint8_t data)
{
	uint8_t flags;

	while (EECR & _BV(EEPE));
	EEAR = addr;
	EEDR = data;
	flags = cli_save_irq();
	EECR = _BV(EEMPE);
	EECR |= _BV(EEPE);
	restore_irq(flags);
}

uint8_t eeprom_read(unsigned addr)
{
	while (EECR & _BV(EEPE));
	EEAR = addr;
	EECR |= _BV(EERE);
	return EEDR;
}
