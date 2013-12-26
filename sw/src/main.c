#define UART_BAUD 9600

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "interrupt.h"
#include "owi.h"
#include "lcd.h"
#include "version.h"

#define HEAT_DDR  DDRC
#define HEAT_PORT PORTC
#define HEAT_BIT  PC5

#define KEY_DDR		DDRD
#define KEY_PORT	PORTD
#define KEY_PIN		(PIND ^ KEY_MASK)
#define KEY_PCIE	_BV(PCIE2)
#define KEY_PCMSK	PCMSK2
#define KEY_VECTOR	PCINT2_vect

#define KEY_PLUS	_BV(PD4)
#define KEY_MINUS	_BV(PD5)
#define KEY_ACCEPT	_BV(PD6)
#define KEY_CANCEL	_BV(PD7)
#define KEY_MASK	(KEY_PLUS | KEY_MINUS | KEY_ACCEPT | KEY_CANCEL)

static uint8_t pending_irq;

void set_pending_irq(uint8_t irq)
{
	pending_irq |= irq;
}

void eeprom_write(unsigned addr, uint8_t data)
{
	uint8_t flags;

	while (EECR & _BV(EEPE));
	EEAR = addr;
	EEDR = data;
	flags = cli_save_irq();
	EECR = _BV(EEMPE);
	EECR |= _BV(EEPE);
	restore_irq(flags);
}

uint8_t eeprom_read(unsigned addr)
{
	while (EECR & _BV(EEPE));
	EEAR = addr;
	EECR |= _BV(EERE);
	return EEDR;
}

static void uart_init(void)
{
	UBRR0 = F_CPU / 8 / UART_BAUD - 1;
	UCSR0A = _BV(U2X0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
}

static void uart_putc(int c)
{
	if (c == '\n')
		uart_putc('\r');
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = c;
}

static void uart_puts(const char *s)
{
	while (*s)
		uart_putc(*s++);
}

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

static void key_init(void)
{
	KEY_DDR &= ~KEY_MASK;
	KEY_PORT |= KEY_MASK;

	KEY_PCMSK |= KEY_MASK;
	PCICR |= KEY_PCIE;
}

ISR(KEY_VECTOR)
{
	set_pending_irq(IRQ_KEY);
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

	HEAT_DDR |= _BV(HEAT_BIT);
	HEAT_PORT |= _BV(HEAT_BIT);
	_delay_ms(1000);
	HEAT_PORT &= ~_BV(HEAT_BIT);

	_delay_ms(2000);
	lcd_clear();

	while(1) {
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

			if (t >= T(41)) {
				if (on)
					uart_puts("Cooling\n");
				on = 0;
			}
			if (t < T(39.5)) {
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
