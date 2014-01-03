#include <stddef.h>
#include "interrupt.h"
#include "timer.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define TIMER_TOP ((F_CPU) / 8000L) //for 1 ms with prescaler 8

struct timer {
	uint32_t time;
	void (*f)(void *p);
	void *p;
};

static volatile uint32_t ms_timer;
static uint32_t timer_min = 0xffffffff;
static struct timer timer[20];

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
	if (++ms_timer >= timer_min)
		set_pending_irq(IRQ_TIMER);
}

uint32_t timer_get_time(void)
{
	uint32_t res;
	uint8_t flags;

	flags = cli_save_irq();
	res = ms_timer;
	restore_irq(flags);

	return res;
}

void timer_process_timers(void)
{
	uint32_t now;

	while ((now = timer_get_time()) >= timer_min) {
		uint32_t min = 0xffffffff;
		uint8_t i;

		for (i = 0; i < ARRAY_SIZE(timer); ++i) {
			if (timer[i].f && timer[i].time <= now) {
				void (*f)(void *) = timer[i].f;

				timer[i].f = NULL;
				f(timer[i].p);
			}
		}
		for (i = 0; i < ARRAY_SIZE(timer); ++i) {
			if (timer[i].f && timer[i].time < min)
				min = timer[i].time;
		}
		timer_min = min;
	}
}

uint8_t timer_add(uint32_t timeout, void (*f)(void *p), void *p)
{
	uint8_t i;

	for (i = 0; i < ARRAY_SIZE(timer); ++i) {
		if (!timer[i].f) {
			timer[i].f = f;
			timer[i].p = p;
			timer[i].time = timer_get_time() + timeout;
			if (timer[i].time < timer_min)
				timer_min = timer[i].time;
			return i;
		}
	}
	return TIMER_INVALID;
}

void timer_del(uint8_t id)
{
	if (id < ARRAY_SIZE(timer))
		timer[id].f = NULL;
}
