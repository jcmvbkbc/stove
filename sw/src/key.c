#include <stddef.h>
#include "interrupt.h"
#include "key.h"
#include "timer.h"

#define KEY_DDR		DDRD
#define KEY_PORT	PORTD
#define KEY_PIN		(PIND ^ KEY_MASK)
#define KEY_PCIE	_BV(PCIE2)
#define KEY_PCMSK	PCMSK2
#define KEY_VECTOR	PCINT2_vect

static uint8_t key_state;
static uint8_t key_auto;
static uint8_t key_timer = TIMER_INVALID;

static key_listener_t *key_listener;
static void *key_context;

void key_init(void)
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

void key_set_listener(key_listener_t *f, void *p)
{
	key_listener = f;
	key_context = p;
}

static void key_auto_repeat(void *p)
{
	if (key_listener)
		key_listener(key_auto, key_state, key_context);
	key_timer = timer_add(30, key_auto_repeat, p);
}

void key_process_keys(void)
{
	uint8_t new_state = KEY_PIN & KEY_MASK;

#if 0
	lcd_puts_xy(8, 1, (new_state & KEY_PLUS) ? "+" : " ");
	lcd_puts_xy(9, 1, (new_state & KEY_MINUS) ? "-" : " ");
	lcd_puts_xy(10, 1, (new_state & KEY_ACCEPT) ? "A" : " ");
	lcd_puts_xy(11, 1, (new_state & KEY_CANCEL) ? "C" : " ");
#endif

	if (key_state != new_state) {
		uint8_t pressed = new_state & ~key_state;

		timer_del(key_timer);

		if (!pressed && new_state) {
			pressed = new_state;
		}
		if (pressed) {
			pressed &= -pressed;
			key_auto = pressed;
			if (key_listener)
				key_listener(key_auto, new_state, key_context);
			key_timer = timer_add(500, key_auto_repeat, NULL);
		}
		key_state = new_state;
	}
}
