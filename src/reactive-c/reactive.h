#ifndef __REACTIVE_H
#define __REACTIVE_H

#include "macros.h"

// let's give the unknown types a name
typedef void* unknown_t;

// we're only exposing the observable type, the inner workings are private
typedef struct observable *observable_t;

// and a (linked) list for observables. use each() & co to create the list.
typedef struct observables *observables_t;

// an observation consists of:
// 1. an array of pointers to the values of the observed observables.
// 2. a pointer to the location of the resulting observing value
typedef struct observation {
  unknown_t *observeds;
  unknown_t observer;
} *observation_t;

// an observer handles an observation
typedef void (*observer_t)(observation_t);

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
observable_t __observing_value(unknown_t);

// actually starts an observable
observable_t start(observable_t);

// suspends a started observable
observable_t suspend(observable_t);

// delay an observable until its scripting parent is activated
observable_t delay(observable_t);

// add a callback to the observable, triggered when it is disposed
typedef void (*observable_callback_t)(observable_t);
observable_t on_dispose(observable_t, observable_callback_t);
observable_t on_activation(observable_t, observable_callback_t);

// remove an observer from all observeds and releases it entirely
void dispose(observable_t);

// trigger and observable to be updated
void observe_update(observable_t observable);

// generic constructor for combined observables
observable_t __merge(observables_t);

// ... emits true when all observables have emitted at least once
observable_t __all(observables_t);

// ... emits true when at least one observable has emitted at least once
observable_t __any(observables_t);

// support for scripting

// script constructor takes a variable amount of (inactive) observables
observable_t __script(int, ...);

// explictly start a script
observable_t run(observable_t);

// morphing constructor to wait until an observable emits an update
observable_t await(observable_t);

#endif
