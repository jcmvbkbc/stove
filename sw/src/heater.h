#ifndef _HEATER_H
#define _HEATER_H

#include <inttypes.h>

void heater_init(void);
void heater_enable(uint8_t enable);
void heater_on(uint8_t on);
void heater_alarm(uint8_t alarm);

#endif /* _HEATER_H */
