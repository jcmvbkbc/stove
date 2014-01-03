#include "interrupt.h"

static uint8_t pending_irq;

void set_pending_irq(uint8_t irq)
{
	pending_irq |= irq;
}

uint8_t get_pending_irq(void)
{
	uint8_t irq = 0;
	uint8_t flags = cli_save_irq();

	if (pending_irq & IRQ_KEY)
		irq |= IRQ_KEY;
	else if (pending_irq & IRQ_TIMER)
		irq |= IRQ_TIMER;
	pending_irq ^= irq;
	restore_irq(flags);

	return irq;
}

