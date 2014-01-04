#include <stddef.h>

#include "heater.h"
#include "lcd.h"
#include "owi.h"
#include "thermo.h"
#include "thermostat.h"
#include "timer.h"
#include "ui.h"

struct stove_state *thermostat_state;

static thermo_listener_t thermostat_fsm;
static void thermostat_fsm(int t, void *p)
{
	static uint8_t on = 0;
	struct stove_state *state = p;

	if (!state->thermostat_time ||
	    timer_get_time() < state->thermostat_time) {
		if (t == T_UNDEF) {
			heater_alarm(1);
			on = 0;
		} else {
			if (t >= state->thermostat_t) {
				heater_on(0);
				on = 0;
			}
			if (t < state->thermostat_t - T(1)) {
				on = 1;
				heater_on(1);
			}
			heater_alarm(0);
		}
	} else {
		heater_on(0);
		on = 0;

	}
	print_t(t, state->thermostat_t, on);
	print_time(0, 1, timer_get_time(), 1);
	if (state->thermostat_time) {
		lcd_puts_xy(7, 1, "/");
		print_time(8, 1, state->thermostat_time, 0);
	}
}

void thermostat_init(struct stove_state *state)
{
	thermostat_state = state;
}

void thermostat_activate(void)
{
	thermo_set_listener(thermostat_fsm, thermostat_state);
}
