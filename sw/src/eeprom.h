#ifndef _EEPROM_H
#define _EEPROM_H

#include <inttypes.h>

#define EEPROM_CALIBRATION	0
#define EEPROM_STOVE_STATE	16
#define EEPROM_STOVE_PROG	64

void eeprom_write(unsigned addr, uint8_t data);
uint8_t eeprom_read(unsigned addr);

#endif /* _EEPROM_H */
