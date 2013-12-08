/*
 * Restricted set of one wire interface commands.
 * Assuming there's only one DS18B20 powered by an external supply.
 */

#ifndef owi_h
#define owi_h

#define T_UNDEF -100

int get_t(void);

#endif
