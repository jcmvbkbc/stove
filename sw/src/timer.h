#ifndef _TIMER_H
#define _TIMER_H

#include <inttypes.h>

#define TIMER_INVALID 0xff

void timer_init(void);
uint32_t timer_get_time(void);
void timer_set_time(uint32_t time);

void timer_process_timers(void);
uint8_t timer_add(uint32_t timeout, void (*f)(void *p), void *p);
void timer_del(uint8_t id);

#endif
