#ifndef __REACTIVE_H
#define __REACTIVE_H

#include "macros.h"

// we're only exposing the observable type, the inner workings are private
typedef struct observable *observable_t;

// and a (linked) list for observables. use each() & co to create the list.
typedef struct observables *observables_t;

// an observer takes an array of arguments and a pointer to store its result
typedef void (*observer_t)(void**, void*);

// retrieve the current value of the observable
void *observable_value(observable_t);

// NOTE: functions with a double underscore prefix are the actual functions
//       offered by the API, BUT, they should not be called directly. Macros.h
//       provides non-prefixed versions that serve as syntactic sugar.

// turns a variadic list of observables into a linked list of observables
observables_t __each(int,...);

// adds observer to a list of observables, providing memory space for its value,
// based on its size
observable_t __observing(observables_t, observer_t, int);

// construct an observable from a (pointer to a) value
observable_t __observing_value(void*);

// acrually starts and observable
observable_t start(observable_t);

// add a callback to the observable, triggered when it is disposed
typedef void (*observable_callback_t)(observable_t);
observable_t on_dispose(observable_t, observable_callback_t);
observable_t on_activation(observable_t, observable_callback_t);

// remove an observer from all observeds and releases it entirely
void dispose(observable_t);

// trigger and observable to be updated
void observe_update(observable_t observable);

// merge multiple observables in a single observable with interleaved updates
observable_t __merge(observables_t);

// support for scripting

// script constructor takes a variable amount of (inactive) observables
observable_t __script(int, ...);

// explictly start a script
observable_t run(observable_t);

// morphing constructor to wait until an observable emits an update
observable_t await(observable_t);

#endif
