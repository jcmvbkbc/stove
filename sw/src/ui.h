#ifndef _UI_H
#define _UI_H

#include <inttypes.h>

#define UI_HINT_NONE	0x20
#define UI_HINT_ALL	0
#define UI_HINT_PMA	1
#define UI_HINT_AC	2

void format_t(char buf[static 7], int t);
void print_t(int t, int t_target, uint8_t dir_t);
void print_time(uint8_t x, uint8_t y, uint32_t time, uint8_t blink);
void show_hint(uint8_t hint);

#endif /* _UI_H */
