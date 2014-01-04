#ifndef _KEY_H
#define _KEY_H

#include <inttypes.h>
#include <avr/io.h>

#define KEY_PLUS	_BV(PD4)
#define KEY_MINUS	_BV(PD5)
#define KEY_ACCEPT	_BV(PD6)
#define KEY_CANCEL	_BV(PD7)
#define KEY_MASK	(KEY_PLUS | KEY_MINUS | KEY_ACCEPT | KEY_CANCEL)

typedef void (key_listener_t)(uint8_t key, uint8_t keys_state, void *p);

void key_init(void);
void key_process_keys(void);
void key_set_listener(key_listener_t *f, void *p);

#endif /* _KEY_H */
