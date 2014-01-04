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

struct stove_state stove_state = {
	.thermostat_t = T(39),
	.mode = MODE_THERMOSTAT,
};

int main() {
	key_init();

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

	timer_init();

	timer_set_time(stove_state.cur_time);
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
