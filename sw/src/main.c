#define UART_BAUD 9600

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include "owi.h"

#define HEAT_DDR  DDRC
#define HEAT_PORT PORTC
#define HEAT_BIT  PC5

void eeprom_write(unsigned addr, unsigned char data)
{
	while (EECR & _BV(EEPE));
	EEAR = addr;
	EEDR = data;
	EECR = _BV(EEMPE);
	EECR |= _BV(EEPE);
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
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = c;
}

static void uart_puts(const char *s)
{
	while (*s)
		uart_putc(*s++);
}

int main() {
	HEAT_DDR |= _BV(HEAT_BIT);

	uart_init();
	uart_puts("Hello\n");

	while(1) {
		int t = get_t();
		_delay_ms(1000);

		if (t > T(30))
			HEAT_PORT ^= _BV(HEAT_BIT);
	}
}
