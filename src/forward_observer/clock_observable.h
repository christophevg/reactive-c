// clock observable

#ifndef __CLOCK_OBSERVABLE_H
#define __CLOCK_OBSERVABLE_H

#include "observer.h"

// we expose a global observable clock
observable_t clock_observable;

void clock_init(void);
void clock_tick(int);
int  clock_now(void);

#endif
