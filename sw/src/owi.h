/*
 * Restricted set of one wire interface commands.
 * Assuming there's only one DS18B20 powered by an external supply.
 */

#ifndef owi_h
#define owi_h

#include <inttypes.h>

#define T_UNDEF (0x7fff)
#define T(v) ((int)((v) * 16))

uint8_t start_get_t(void);
int read_t(void);
int get_t(void);

#endif
