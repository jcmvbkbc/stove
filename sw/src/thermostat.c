#include <stddef.h>
#include <string.h>

#include "heater.h"
#include "key.h"
#include "lcd.h"
#include "menu.h"
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
static struct stove_prog thermostat_prog;

static uint8_t thermostat_get_point(void)
{
	uint16_t time_min = timer_get_time() / 60000;
	uint8_t i;

	for (i = 0; i < N_POINT; ++i) {
		if (time_min < thermostat_prog.point[i].time)
			break;
	}
	return i;
}

static void thermostat_update_ui(void)
{
	static const uint8_t hint[THERMOSTAT_UI_MAX] = {
		UI_HINT_AC,
		UI_HINT_PMA,
		UI_HINT_PMA,
		UI_HINT_PMA,
	};
	static const uint8_t editor_x[THERMOSTAT_UI_MAX] = {
		16, 11, 12, 4,
	};
	static const uint8_t editor_y[THERMOSTAT_UI_MAX] = {
		0, 0, 1, 1,
	};
	struct stove_state *state = thermostat_state;

	print_time(0, 1, timer_get_time(), 1);
	if (state->mode == MODE_THERMOSTAT) {
		print_t(thermostat_last_t, state->thermostat_t,
			thermostat_heater);
		if (state->thermostat_time) {
			lcd_puts_xy(7, 1, "/");
			print_time(8, 1, state->thermostat_time, 0);
		} else {
			lcd_puts_xy(7, 1, "        ");
		}
	} else {
		uint8_t i = thermostat_get_point();

		if (i < N_POINT) {
			print_t(thermostat_last_t, thermostat_prog.point[i].t,
				thermostat_heater);
			lcd_puts_xy(7, 1, "/");
			print_time(8, 1, thermostat_prog.point[i].time * 60000,
				   0);
		} else {
			print_t(thermostat_last_t, T_UNDEF,
				thermostat_heater);
			lcd_puts_xy(7, 1, "        ");
		}
	}

	show_hint(hint[thermostat_ui_edit]);
	lcd_xy(editor_x[thermostat_ui_edit], editor_y[thermostat_ui_edit]);
}

static void thermostat_control(int t, int target_t)
{
	if (t == T_UNDEF) {
		heater_alarm(1);
		thermostat_heater = 0;
	} else {
		if (t >= target_t) {
			heater_on(0);
			thermostat_heater = 0;
		}
		if (t < target_t - T(1)) {
			thermostat_heater = 1;
			heater_on(1);
		}
		heater_alarm(0);
	}
}

static thermo_listener_t thermostat_fsm;
static void thermostat_fsm(int t, void *p)
{
	struct stove_state *state = p;

	thermostat_last_t = t;
	if (!state->thermostat_time ||
	    timer_get_time() < state->thermostat_time) {
		thermostat_control(t, state->thermostat_t);
	} else {
		heater_alarm(0);
		heater_on(0);
		thermostat_heater = 0;

	}
	thermostat_update_ui();
}

static thermo_listener_t thermostat_prog_fsm;
static void thermostat_prog_fsm(int t, void *p)
{
	struct stove_state *state = p;
	uint8_t i = thermostat_get_point();

	thermostat_last_t = t;

	if (i < N_POINT) {
		thermostat_control(t, thermostat_prog.point[i].t);
	} else {
		heater_alarm(0);
		heater_on(0);
		thermostat_heater = 0;
	}
	thermostat_update_ui();
}

void thermostat_init(struct stove_state *state)
{
	thermostat_state = state;
}

static key_listener_t thermostat_key_fsm;
static void thermostat_key_fsm(uint8_t key, uint8_t keys_state, void *p)
{
	static const uint8_t bad_edit[2][THERMOSTAT_UI_MAX] = {
		{
			[THERMOSTAT_UI_T] = 1,
			[THERMOSTAT_UI_TIME] = 1,
		},
	};
	struct stove_state *state = p;
	int sign = 0;
	int t;

	switch (key) {
	case KEY_PLUS:
		sign = 1;
		break;

	case KEY_MINUS:
		sign = -1;
		break;

	case KEY_ACCEPT:
		state_save(state);
		do {
			if (++thermostat_ui_edit >= THERMOSTAT_UI_MAX) {
				thermostat_ui_edit = 0;
			}
		} while (bad_edit[state->mode == MODE_THERMOSTAT][thermostat_ui_edit]);
		break;

	case KEY_CANCEL:
		if (thermostat_ui_edit == THERMOSTAT_UI_CUR_TIME) {
			state->cur_time = 0;
			timer_set_time(0);
		} else {
			menu_activate();
		}
		break;
	}

	if (sign) {
		switch (thermostat_ui_edit) {
		case THERMOSTAT_UI_T:
			t = (state->thermostat_t * 10) >> 4;
			while (t == (state->thermostat_t * 10) >> 4) {
				state->thermostat_t += sign;
			}
			break;

		case THERMOSTAT_UI_TIME:
			state->thermostat_time += 60000 * sign;
			break;

		case THERMOSTAT_UI_CUR_TIME:
			state->cur_time = timer_get_time() + 60000 * sign;
			timer_set_time(state->cur_time);
			break;

		default:
			break;
		}
	}

	thermostat_update_ui();
}

void thermostat_activate(void)
{
	lcd_page(0);
	if (thermostat_state->mode == MODE_THERMOSTAT) {
		thermo_set_listener(thermostat_fsm, thermostat_state);
		key_set_listener(thermostat_key_fsm, thermostat_state);
	} else {
		if (!state_load_prog(&thermostat_prog, thermostat_state->mode)) {
			memset(&thermostat_prog, 0, sizeof(thermostat_prog));
		}
		thermo_set_listener(thermostat_prog_fsm, thermostat_state);
		key_set_listener(thermostat_key_fsm, thermostat_state);
	}
}
