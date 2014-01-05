#ifndef _LCD_H
#define _LCD_H

#include <inttypes.h>

#define LCD_ON_DISPLAY	0x4
#define LCD_ON_CURSOR	0x2
#define LCD_ON_BLINK	0x1

#define LCD_MODE_ID	0x2
#define LCD_MODE_SH	0x1

#define LCD_SHIFT_RL	0x4
#define LCD_SHIFT_SC	0x8

#define LCD_STRIDE	0x40
#define LCD_ADDR_MASK	0x7f

void lcd_init(void);

void lcd_clear(void);
void lcd_on(uint8_t mode);
void lcd_set_entry_mode(uint8_t mode);

void lcd_xy(uint8_t x, uint8_t y);
void lcd_putc(char c);
void lcd_puts(const char *str);
void lcd_puts_xy(uint8_t x, uint8_t y, const char *str);
void lcd_page(uint8_t page);

#endif
