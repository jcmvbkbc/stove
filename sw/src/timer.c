#include "interrupt.h"
#include "timer.h"

#define TIMER_TOP ((F_CPU) / 8000L) //for 1 ms with prescaler 8

static volatile uint32_t ms_timer;

void timer_init(void)
{
	TCNT0 = 0x00;

	OCR0A = TIMER_TOP;
	TCCR0A |= _BV(WGM01); //clear timer on compare match
	TCCR0B |= _BV(CS01); //prescaler CLKio/8
	TIMSK0 |= _BV(OCIE0A); //compare match interrupt enabled
}

ISR(TIMER0_COMPA_vect)
{
	ms_timer++;
}

uint32_t get_ms(void)
{
	uint32_t res;
	uint8_t flags;

	flags = cli_save_irq();
	res = ms_timer;
	restore_irq(flags);

	return res;
}
