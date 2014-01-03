#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "heater.h"
#include "interrupt.h"
#include "owi.h"
#include "key.h"
#include "lcd.h"
#include "timer.h"
#include "uart.h"
#include "version.h"

static void print_t(int t)
{
	char buf[10];
	char *p = buf + sizeof(buf) - 1;
	int i;
	int sign = t < 0;

	if (sign)
		t = -t;
	*p = 0;
	t *= 10;
	t >>= 4;
	for (i = 0; t; ++i, t /= 10) {
		*--p = '0' + (t % 10);
		if (!i)
			*--p = '.';
	}
	if (*p == '.')
		*--p = '0';
	if (sign)
		*--p = '-';
	uart_puts(p);
	lcd_puts_xy(10, 0, p);
	lcd_puts("\x99""C   ");
}

int main() {
	unsigned char on = 0;
	unsigned char off = 0;

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

	while (1) {
		int t = get_t();

		if (t == T_UNDEF) {
			uart_puts("Thermo disconnected\n");
			HEAT_PORT |= _BV(HEAT_BIT);
			_delay_ms(500);
			HEAT_PORT &= ~_BV(HEAT_BIT);
			_delay_ms(500);
			HEAT_PORT |= _BV(HEAT_BIT);
			_delay_ms(500);
			HEAT_PORT &= ~_BV(HEAT_BIT);
			_delay_ms(30000);
		} else {
			uart_puts("T: ");
			print_t(t);
			uart_puts("\n");

			if (t >= T(39)) {
				if (on)
					uart_puts("Cooling\n");
				on = 0;
			}
			if (t < T(38)) {
				if (!on)
					uart_puts("Heating\n");
				on = 1;
			}
			if (on || off > 30) {
				HEAT_PORT |= _BV(HEAT_BIT);
				off = 0;
			} else {
				HEAT_PORT &= ~_BV(HEAT_BIT);
				off += 3;
			}
			_delay_ms(3000);
		}
	}
}
