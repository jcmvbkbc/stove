#ifndef _STATE_H
#define _STATE_H

#include <inttypes.h>

#define MODE_THERMOSTAT 0xff

#define N_PROG	7
#define N_POINT	15

struct stove_point {
	uint16_t time; /* point time (minutes) */
	int16_t t; /* point temperature */
};

struct stove_prog {
	struct stove_point point[N_POINT];
	uint32_t crc;
};

struct stove_state {
	uint32_t cur_time; /* session runtime */
	uint32_t thermostat_time; /* total session time for thermostat mode, 0 == no limit */
	int16_t thermostat_t; /* thermostat target temperature */
	uint8_t mode; /* program index, or MODE_THERMOSTAT */
	uint8_t n_prog; /* number of predefined programs */
	uint32_t crc;
};

uint8_t state_load(struct stove_state *state);
uint8_t state_save(struct stove_state *state);

uint8_t state_load_prog(struct stove_prog *prog, uint8_t idx);
uint8_t state_save_prog(struct stove_prog *prog, uint8_t idx);

#endif /* _STATE_H */
