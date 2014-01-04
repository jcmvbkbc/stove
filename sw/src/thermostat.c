#include <stddef.h>

#include "heater.h"
#include "lcd.h"
#include "owi.h"
#include "thermo.h"
#include "thermostat.h"
#include "timer.h"
#include "ui.h"

struct stove_state *thermostat_state;
static int thermostat_last_t;
static uint8_t thermostat_heater;
enum thermostat_ui_item {
	THERMOSTAT_UI_NONE,
	THERMOSTAT_UI_T,
	THERMOSTAT_UI_TIME,
	THERMOSTAT_UI_CUR_TIME,
	THERMOSTAT_UI_MAX
};
static uint8_t thermostat_ui_edit;

static void thermostat_update_ui(void)
{
	static const uint8_t editor_x[THERMOSTAT_UI_MAX] = {
		16, 11, 12, 4,
	};
	static const uint8_t editor_y[THERMOSTAT_UI_MAX] = {
		0, 0, 1, 1,
	};
	struct stove_state *state = thermostat_state;

	print_t(thermostat_last_t, state->thermostat_t, thermostat_heater);
	print_time(0, 1, timer_get_time(), 1);
	if (state->thermostat_time) {
		lcd_puts_xy(7, 1, "/");
		print_time(8, 1, state->thermostat_time, 0);
	}

	lcd_xy(editor_x[thermostat_ui_edit], editor_y[thermostat_ui_edit]);
}

static thermo_listener_t thermostat_fsm;
static void thermostat_fsm(int t, void *p)
{
	struct stove_state *state = p;

	thermostat_last_t = t;
	if (!state->thermostat_time ||
	    timer_get_time() < state->thermostat_time) {
		if (t == T_UNDEF) {
			heater_alarm(1);
			thermostat_heater = 0;
		} else {
			if (t >= state->thermostat_t) {
				heater_on(0);
				thermostat_heater = 0;
			}
			if (t < state->thermostat_t - T(1)) {
				thermostat_heater = 1;
				heater_on(1);
			}
			heater_alarm(0);
		}
	} else {
		heater_on(0);
		thermostat_heater = 0;

	}
	thermostat_update_ui();
}

void thermostat_init(struct stove_state *state)
{
	thermostat_state = state;
}

void thermostat_activate(void)
{
	thermo_set_listener(thermostat_fsm, thermostat_state);
}