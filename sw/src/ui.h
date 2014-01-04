#ifndef _UI_H
#define _UI_H

#include <inttypes.h>

void format_t(char buf[static 7], int t);
void print_t(int t, int t_target, uint8_t dir_t);
void print_time(uint8_t x, uint8_t y, uint32_t time, uint8_t blink);

#endif /* _UI_H */
