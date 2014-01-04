#include "thermo.h"
#include "timer.h"

struct thermo {
	int t;
	uint8_t state;
	uint8_t timer;
};

static struct thermo thermo = {
	.timer = TIMER_INVALID,
};
static thermo_listener_t *thermo_listener;
static void *thermo_listener_arg;

static void thermo_fsm(void *p)
{
	struct thermo *thermo = p;

	thermo->timer = timer_add(1000, thermo_fsm, thermo);
	thermo->t = thermo->state ? read_t() : T_UNDEF;
	thermo->state = start_get_t();

	if (thermo_listener)
		thermo_listener(thermo->t, thermo_listener_arg);
}

void thermo_init(void)
{
	thermo_fsm(&thermo);
}

void thermo_set_listener(thermo_listener_t *f, void *p)
{
	thermo_listener = f;
	thermo_listener_arg = p;
}
