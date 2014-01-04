#ifndef _THERMOSTAT_H
#define _THERMOSTAT_H

#include "state.h"

void thermostat_init(struct stove_state *state);
void thermostat_activate(void);

#endif /* _THERMOSTAT_H */
