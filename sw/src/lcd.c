#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

#define LCD_CTRL_DDR	DDRB
#define LCD_CTRL_PORT	PORTB
#define LCD_RS		_BV(PB1)
#define LCD_RW		_BV(PB2)
#define LCD_E		_BV(PB6)
#define LCD_CTRL_MASK	(LCD_RS | LCD_RW | LCD_E)

#define LCD_DATA_DDR	DDRC
#define LCD_DATA_PORT	PORTC
#define LCD_DATA_SHIFT	0
#define LCD_DATA_MASK	0xf

static uint8_t lcd_addr;

static inline void lcd_prepare_write(uint8_t addr)
{
	LCD_CTRL_PORT = (LCD_CTRL_PORT & ~LCD_CTRL_MASK) |
		((addr & 1) ? LCD_RS : 0);
	LCD_DATA_DDR |= LCD_DATA_MASK;
	_delay_us(1);
}

static void lcd_write_nibble(uint8_t v)
{
	LCD_CTRL_PORT |= LCD_E;
	LCD_DATA_PORT &= ~LCD_DATA_MASK;
	LCD_DATA_PORT |= (v << LCD_DATA_SHIFT) & LCD_DATA_MASK;
	_delay_us(1);
	LCD_CTRL_PORT &= ~LCD_E;
	_delay_us(1);
}

static void lcd_write(uint8_t addr, uint8_t v)
{
	lcd_prepare_write(addr);
	lcd_write_nibble(v >> 4);
	lcd_write_nibble(v);
}

void lcd_clear(void)
{
	lcd_write(0, 1);
	_delay_ms(2);
	lcd_addr = 0;
}

void lcd_on(uint8_t mode)
{
	lcd_write(0, 0x8 | (mode & 7));
	_delay_us(40);
}

void lcd_set_entry_mode(uint8_t mode)
{
	lcd_write(0, 0x4 | (mode & 3));
	_delay_us(40);
}

static inline void lcd_set_addr(uint8_t addr)
{
	lcd_addr = addr & LCD_ADDR_MASK;
	lcd_write(0, 0x80 | lcd_addr);
	_delay_us(40);
}

void lcd_init(void)
{
	LCD_CTRL_PORT &= ~LCD_CTRL_MASK;
	LCD_CTRL_DDR |= LCD_CTRL_MASK;

	_delay_ms(20);
	lcd_prepare_write(0);
	lcd_write_nibble(0x3);
	_delay_us(40);
	lcd_write_nibble(0x3);
	_delay_us(40);
	lcd_write_nibble(0x3);
	_delay_us(40);
	lcd_write_nibble(0x2);
	_delay_us(40);
	lcd_write(0, 0x28);
	_delay_us(40);
	lcd_on(0);
	lcd_clear();
	lcd_set_entry_mode(LCD_MODE_ID);
	lcd_on(LCD_ON_DISPLAY | LCD_ON_CURSOR);
}

void lcd_xy(uint8_t x, uint8_t y)
{
	lcd_set_addr((x & (LCD_STRIDE - 1)) + LCD_STRIDE * y);
}

void lcd_putc(char c)
{
	lcd_write(1, c);
	_delay_us(40);
	lcd_addr = (lcd_addr + 1) & LCD_ADDR_MASK;
}

void lcd_puts(const char *str)
{
	uint8_t x = lcd_addr & (LCD_STRIDE - 1);

	for (; x < LCD_STRIDE && *str; ++x, ++str) {
		lcd_putc(*str);
	}
}

void lcd_puts_xy(uint8_t x, uint8_t y, const char *str)
{
	for (lcd_xy(x, y); x < LCD_STRIDE && *str; ++x, ++str) {
		lcd_putc(*str);
	}
}


