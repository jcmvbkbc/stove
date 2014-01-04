#include <stddef.h>
#include "eeprom.h"
#include "state.h"

#define CRC_POLY 0xedb88320
#define EEPROM_STOVE_STATE 16
#define EEPROM_STOVE_PROG  64

static uint32_t crc(const void *p, uint8_t sz)
{
	uint8_t i, j;
	uint32_t crc = ~0;

	for (i = 0; i < sz; ++i) {
		crc ^= ((uint8_t *)p)[i];
		for (j = 0; j < 8; ++j)
			if (crc & 1)
				crc = (crc >> 1) ^ CRC_POLY;
			else
				crc >>= 1;
	}
	return ~crc;
}

static void generic_save(const void *p, uint8_t sz, uint16_t base)
{
	uint8_t i;

	for (i = 0; i < sz; ++i) {
		uint8_t v = ((uint8_t *)p)[i];

		if (eeprom_read(base + i) != v)
			eeprom_write(base + i, v);
	}
}

static void generic_load(void *p, uint8_t sz, uint16_t base)
{
	uint8_t i;

	for (i = 0; i < sz; ++i)
		((uint8_t *)p)[i] = eeprom_read(base + i);
}

uint8_t state_load(struct stove_state *state)
{
	generic_load(state, sizeof(*state), EEPROM_STOVE_STATE);
	return crc(state, offsetof(struct stove_state, crc)) == state->crc;
}

uint8_t state_save(struct stove_state *state)
{
	state->crc = crc(state, offsetof(struct stove_state, crc));
	generic_save(state, sizeof(*state), EEPROM_STOVE_STATE);
	return state_load(state);
}

uint8_t state_load_prog(struct stove_prog *prog, uint8_t idx)
{
	generic_load(prog, sizeof(*prog),
		     EEPROM_STOVE_PROG + sizeof(*prog) * idx);
	return crc(prog, offsetof(struct stove_prog, crc)) == prog->crc;
}

uint8_t state_save_prog(struct stove_prog *prog, uint8_t idx)
{
	prog->crc = crc(prog, offsetof(struct stove_prog, crc));
	generic_save(prog, sizeof(*prog),
		     EEPROM_STOVE_PROG + sizeof(*prog) * idx);
	return state_load_prog(prog, idx);
}
