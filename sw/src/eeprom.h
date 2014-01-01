#ifndef _EEPROM_H
#define _EEPROM_H

#include <inttypes.h>

void eeprom_write(unsigned addr, uint8_t data);
uint8_t eeprom_read(unsigned addr);

#endif /* _EEPROM_H */
