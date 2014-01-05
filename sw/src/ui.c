#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "owi.h"
#include "ui.h"

void format_t(char buf[static 7], int t)
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

void print_t(int t, int t_target, uint8_t dir_t)
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

void print_time(uint8_t x, uint8_t y, uint32_t time, uint8_t blink)
{
	char time_str[10];
	unsigned time_min;
	uint8_t colon;

	time /= 1000;
	colon = (time & 1) || !blink;
	time_min = time / 60;

	sprintf(time_str, "%02d%c%02d",
		time_min / 60, colon ? ':' : ' ', time_min % 60);
	lcd_puts_xy(x, y, time_str);
}

void show_hint(uint8_t hint)
{
	lcd_xy(15, 1);
	lcd_putc(hint);
}
