#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <avr/interrupt.h>
#include <inttypes.h>

enum {
	IRQ_KEY		= 0x1,
	IRQ_TIMER	= 0x2,
};

void set_pending_irq(uint8_t irq);

static inline uint8_t cli_save_irq(void)
{
	uint8_t flags = SREG;
	cli();
	return flags;
}

static inline void restore_irq(uint8_t flags)
{
	SREG = flags;
}

#endif /* _INTERRUPT_H */
