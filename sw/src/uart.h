#ifndef _UART_H
#define _UART_H

void uart_init(void);
void uart_putc(int c);

static inline void uart_puts(const char *s)
{
	while (*s)
		uart_putc(*s++);
}

#endif /* _UART_H */
