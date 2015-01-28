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
void dispose(observable_t);

// trigger and observable to be updated
void observe_update(observable_t observable);

// merge multiple observables, resulting in a single observable with interleaved
// updates
observable_t merge(observable_li_t);

// maps an observable to something else ...
observable_t map(observable_t, observer_t, int);

observable_t addi(observable_t, observable_t);
observable_t addd(observable_t, observable_t);

// demo for lifting through macro-expansion
#define lift2(type, fun) \
  void lifted_##fun(void **in, void *out) { \
    *(type*)(out) = fun((*(type*)(in[0])), (*(type*)(in[1]))); \
  }

// support for scripting

typedef struct fragment *fragment_t;

fragment_t await(observable_t);

observable_t observable_from_script(fragment_t, fragment_t);

#endif
