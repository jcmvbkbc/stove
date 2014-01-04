#include <inttypes.h>
#include <stddef.h>
#include <avr/io.h>
#include <util/delay.h>

#include "heater.h"
#include "interrupt.h"
#include "key.h"
#include "lcd.h"
#include "owi.h"
#include "state.h"
#include "thermo.h"
#include "thermostat.h"
#include "timer.h"
#include "uart.h"
#include "version.h"
#include "ui.h"

struct stove_state stove_state;

static void state_record(void *p)
{
	timer_add(600000ul, state_record, p);
	stove_state.cur_time = timer_get_time();
	if (!state_save(&stove_state))
		lcd_puts_xy(15, 1, "E");
	else
		lcd_puts_xy(15, 1, " ");
}

int main() {
	uart_init();
	uart_puts("Hello\n");
	uart_puts("v.");
	uart_puts(version);
	uart_puts("\n");

	lcd_init();
	lcd_puts_xy(0, 0, "Hello!");
	lcd_puts_xy(0, 1, "v.");
	lcd_puts(version);

	heater_init();

	_delay_ms(2000);
	lcd_clear();

	key_init();
	timer_init();

	if (!state_load(&stove_state)) {
		lcd_puts_xy(15, 1, "L");
		stove_state = (struct stove_state){
			.thermostat_t = T(39),
			.mode = MODE_THERMOSTAT,
		};
	}
	timer_set_time(stove_state.cur_time);
	state_record(&stove_state);
	thermostat_init(&stove_state);

	sei();

	heater_enable(1);

	thermo_init();
	if (stove_state.mode == MODE_THERMOSTAT)
		thermostat_activate();

	while (1) {
		switch (get_pending_irq()) {
		case IRQ_KEY:
			key_process_keys();
			break;

		case IRQ_TIMER:
			timer_process_timers();
			break;
		}
	}
}
