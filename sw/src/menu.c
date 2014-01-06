#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "key.h"
#include "lcd.h"
#include "menu.h"
#include "thermostat.h"
#include "timer.h"
#include "ui.h"

#define MENU_MAX_DEPTH 5

static struct stove_state *menu_state;
static uint8_t menu_level;
static uint8_t menu_location[MENU_MAX_DEPTH];

static void menu_thermostat_continue(void);

static void menu_prog_activate(void);
static void menu_prog_start(void);
static void menu_prog_continue(void);

static struct menu_item *menu_prog_select(uint8_t i);
static struct menu_item *menu_prog_edit(uint8_t i);

struct menu;
struct menu_item {
	const char *name;
	const struct menu *sub;
	void (*f)(void);
};

struct menu {
	struct menu_item *item;
	struct menu_item *(*get_item)(uint8_t i);
	void (*activate)(void);
};

static const struct menu menu_root = {
	.item = (struct menu_item []){
		{
			.name = "Thermostat",
			.f = menu_thermostat_continue,
		},
		{
			.name = "Program",
			.sub = &(struct menu){
				.get_item = menu_prog_select,
			},
		},
		{
			.name = NULL,
		},
	},
};

static const struct menu menu_prog = {
	.item = (struct menu_item[]){
		{
			.name = "Continue",
			.f = menu_prog_continue,
		},
		{
			.name = "Start",
			.f = menu_prog_start,
		},
		{
			.name = "Edit",
			.sub = &(struct menu){
				.get_item = menu_prog_edit,
			},
		},
		{
			.name = NULL,
		},
	},
	.activate = menu_prog_activate,
};

static const struct menu *menu_current = &menu_root;

static inline struct menu_item *menu_current_item(uint8_t i)
{
	struct menu_item *item = menu_current->item;

	if (item)
		return item + i;
	else
		return menu_current->get_item(i);
}

static void menu_current_update(void)
{
	uint8_t i;

	menu_current = &menu_root;
	for (i = 0; i < menu_level; ++i)
		menu_current = menu_current_item(menu_location[i])->sub;
}

static void menu_update_ui(void)
{
	uint8_t i;

	for (i = 0; i < 2; ++i) {
		uint8_t j = menu_location[menu_level] + i;
		char str[17];
		struct menu_item *item = menu_current_item(j);

		if (item->name) {
			snprintf(str, sizeof(str), "%15.15s%c",
				 item->name,
				 item->sub ? '\x84' :
				 (item->f ? '\x10' : ' '));
		} else {
			memset(str, ' ', sizeof(str));
			str[sizeof(str) - 1] = 0;
		}
		lcd_puts_xy(16, i, str);
	}
	lcd_xy(16, 0);
}

static key_listener_t menu_key_fsm;
static void menu_key_fsm(uint8_t key, uint8_t keys_state, void *p)
{
	uint8_t i = menu_location[menu_level];
	struct menu_item *item = menu_current_item(i);

	switch (key) {
	case KEY_PLUS:
		if (i)
			--menu_location[menu_level];
		break;

	case KEY_MINUS:
		if (menu_current_item(i + 1)->name)
			++menu_location[menu_level];
		break;

	case KEY_ACCEPT:
		if (item->sub) {
			++menu_level;
			menu_current_update();
			menu_location[menu_level] = 0;
		} else if (item->f) {
			item->f();
			return;
		}
		break;

	case KEY_CANCEL:
		if (menu_level > 0) {
			--menu_level;
			menu_current_update();
		} else {
			thermostat_activate();
			return;
		}
		break;
	}
	menu_update_ui();
}

void menu_init(struct stove_state *state)
{
	menu_state = state;
}

void menu_activate(void)
{
	lcd_page(1);
	key_set_listener(menu_key_fsm, NULL);
	menu_update_ui();
}

static void menu_thermostat_continue(void)
{
	menu_state->mode = MODE_THERMOSTAT;
	state_save(menu_state);
	thermostat_activate();
}


static uint8_t current_prog_idx;
static struct stove_prog current_prog;

static void menu_prog_activate(void)
{
	current_prog_idx = menu_location[menu_level - 1];
	if (!state_load_prog(&current_prog, current_prog_idx))
		memset(&current_prog, 0, sizeof(current_prog));
}

static void menu_prog_start(void)
{
	menu_state->cur_time = 0;
	timer_set_time(0);
	menu_prog_continue();
}

static void menu_prog_continue(void)
{
	menu_state->mode = current_prog_idx;
	state_save(menu_state);
	thermostat_activate();
}

static void menu_prog_save(void)
{
	if (state_save_prog(&current_prog, current_prog_idx)) {
		--menu_level;
		menu_current_update();
		menu_update_ui();
	}
}

static struct menu_item *menu_prog_select(uint8_t i)
{
	static struct menu_item item;

	if (i < N_PROG) {
		struct stove_prog prog;
		static char name[17];
		char t[7];

		if (!state_load_prog(&prog, i))
			memset(&prog, 0, sizeof(prog));
		format_t(t, prog.point[0].t);
		snprintf(name, sizeof(name), "%d %02d:%02d %5.5s\x99""C",
			 i + 1, prog.point[0].time / 60,
			 prog.point[0].time % 60, t);

		item.name = name;
		item.sub = &menu_prog;
	} else {
		item.name = NULL;
	}
	return &item;
}

static void menu_prog_point_edit(void)
{
}

static struct menu_item *menu_prog_edit(uint8_t i)
{
	static struct menu_item item;

	if (i < N_POINT) {
		static char name[17];
		char t[7];

		format_t(t, current_prog.point[i].t);
		snprintf(name, sizeof(name), "%d %02d:%02d %5.5s\x99""C",
			 i + 1, current_prog.point[i].time / 60,
			 current_prog.point[i].time % 60, t);

		item.name = name;
		item.f = menu_prog_point_edit;
	} else if (i == N_POINT) {
		item.name = "Save";
		item.f = menu_prog_save;
	} else {
		item.name = NULL;
	}
	return &item;
}
