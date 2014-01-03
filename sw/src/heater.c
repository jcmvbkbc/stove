#include <avr/io.h>
#include <util/delay.h>
#include "heater.h"
#include "timer.h"

#define HEAT_DDR  DDRC
#define HEAT_PORT PORTC
#define HEAT_BIT  PC5

static uint8_t heater_is_enabled;
static uint8_t heater_is_on;
static uint8_t heater_is_alarmed;
static uint8_t heater_timer = TIMER_INVALID;

void heater_init(void)
{
	HEAT_DDR |= _BV(HEAT_BIT);
	HEAT_PORT |= _BV(HEAT_BIT);
	_delay_ms(1000);
	HEAT_PORT &= ~_BV(HEAT_BIT);
}

static void heater_alarm_fsm(void *p)
{
	uint8_t i = (intptr_t)p;

	++i;

	if (i & 1)
		HEAT_PORT &= ~_BV(HEAT_BIT);
	else
		HEAT_PORT |= _BV(HEAT_BIT);

	if (i >= 4) {
		i = 0;
	}

	heater_timer = timer_add(i < 3 ? 500 : 28500,
				 heater_alarm_fsm, (void *)(intptr_t)i);
}

static void heater_off_fsm(void *p)
{
	uint8_t i = (intptr_t)p;

	i ^= 1;

	if (i)
		HEAT_PORT |= _BV(HEAT_BIT);
	else
		HEAT_PORT &= ~_BV(HEAT_BIT);

	heater_timer = timer_add(i ? 3000 : 27000,
				 heater_off_fsm, (void *)(intptr_t)i);
}

static void heater_update(void)
{
	timer_del(heater_timer);
	heater_timer = TIMER_INVALID;

	if (heater_is_enabled) {
		if (heater_is_alarmed) {
			HEAT_PORT |= _BV(HEAT_BIT);
			heater_timer = timer_add(500, heater_alarm_fsm,
						 (void *)0);
		} else if (heater_is_on) {
			HEAT_PORT |= _BV(HEAT_BIT);
		} else {
			HEAT_PORT &= ~_BV(HEAT_BIT);
			heater_timer = timer_add(30000, heater_off_fsm,
						 (void *)0);
		}
	} else {
		HEAT_PORT &= ~_BV(HEAT_BIT);
	}
}

void heater_enable(uint8_t enable)
{
	if (heater_is_enabled != enable) {
		heater_is_enabled = enable;
		heater_update();
	}
}

void heater_on(uint8_t on)
{
	if (heater_is_on != on) {
		heater_is_on = on;
		if (!heater_is_alarmed)
			heater_update();
	}
}

void heater_alarm(uint8_t alarm)
{
	if (heater_is_alarmed != alarm) {
		heater_is_alarmed = alarm;
		heater_update();
	}
}
