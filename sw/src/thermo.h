#ifndef _THERMO_H
#define _THERMO_H

#include "owi.h"

typedef void (thermo_listener_t)(int t, void *p);

void thermo_init(void);
void thermo_set_listener(thermo_listener_t *f, void *p);

#endif /* _THERMO_H */
