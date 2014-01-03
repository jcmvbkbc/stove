#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "heater.h"
#include "interrupt.h"
#include "owi.h"
#include "key.h"
#include "lcd.h"
#include "timer.h"
#include "uart.h"
#include "version.h"

static void format_t(char buf[static 7], int t)
{
	if (t == T_UNDEF) {
		strcpy(buf, "??.?");
	} else {
		uint8_t sign = t < 0;

		if (sign) {
			t = -t;
			*buf++ = '-';
		}
		t *= 10;
		t >>= 4;
		sprintf(buf, "%d.%d", t / 10, t % 10);
	}
}

static void print_t(int t, int t_target, uint8_t dir_t)
{
	char t_buf[8];
	char t_target_buf[8];
	char buf[20];
	static const char dir[2] = {
		0xda, 0xd9
	};

	format_t(t_buf, t);
	format_t(t_target_buf, t_target);
	sprintf(buf, "%5s %c%5s\x99""C", t_buf, dir[dir_t], t_target_buf);
	lcd_puts_xy(0, 0, buf);
}

static void print_time(uint32_t time)
{
	char time_str[10];
	unsigned time_min;
	uint8_t colon;

	colon = time & 1;
	time_min = time / 60;

	sprintf(time_str, "%02d%c%02d",
		time_min / 60, colon ? ':' : ' ', time_min % 60);
	lcd_puts_xy(0, 1, time_str);
}

struct thermo {
	int t;
	uint8_t state;
	uint8_t timer;
};

static void thermostat_fsm(int t)
{
	static uint8_t on = 0;

	if (t == T_UNDEF) {
		heater_alarm(1);
		on = 0;
	} else {
		if (t >= T(39)) {
			heater_on(0);
			on = 0;
		}
		if (t < T(38)) {
			on = 1;
			heater_on(1);
		}
		heater_alarm(0);
	}
	print_t(t, T(39), on);
	print_time(timer_get_time() / 1000);
}

static void thermo_fsm(void *p)
{
	struct thermo *thermo = p;

	thermo->timer = timer_add(1000, thermo_fsm, thermo);
	thermo->t = thermo->state ? read_t() : T_UNDEF;
	thermo->state = start_get_t();

	thermostat_fsm(thermo->t);
}

int main() {
	struct thermo thermo = {
		.state = 0,
	};

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

	sei();

	heater_enable(1);

	thermo_fsm(&thermo);

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
