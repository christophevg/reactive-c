#ifndef __REACTIVE_H
#define __REACTIVE_H

typedef struct observable *observable_t;
typedef void (*observer_t)(void*, void*);

// construct an observable from a (pointer to a) value
observable_t observable_from_value(void*);

// retrieve the current value of the observable
void *observable_value(observable_t);

observable_t observe(observable_t, observer_t, int);

void observe_update(observable_t observable);

#endif
