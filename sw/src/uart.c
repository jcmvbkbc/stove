#include <avr/io.h>

#define UART_BAUD 9600

void uart_init(void)
{
	UBRR0 = F_CPU / 8 / UART_BAUD - 1;
	UCSR0A = _BV(U2X0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
}

void uart_putc(int c)
{
	if (c == '\n')
		uart_putc('\r');
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = c;
}
