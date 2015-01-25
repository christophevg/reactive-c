#ifndef __REACTIVE_H
#define __REACTIVE_H

#include <stdbool.h>

// we're only exposing the observable type, the inner workings are private
typedef struct observable *observable_t;

// linked list item (LI) for observables. use all to create a list.
typedef struct observable_li *observable_li_t;

// an observer takes an array of arguments and a pointer to store its result
typedef void (*observer_t)(void**, void*);

// construct an observable from a (pointer to a) value
observable_t observable_from_value(void*);

// retrieve the current value of the observable
void *observable_value(observable_t);

// turns a variadic list of observables into a linked list of observables
observable_li_t all(int,...);

// adds observers to a list of observables, providing memory space for its
// value, based on its size
observable_t observe(observable_li_t, observer_t, int);

// removed an observer from all observeds and releases it entirely
bool dispose(observable_t);

// trigger and observable to be updated
void observe_update(observable_t observable);

#endif
