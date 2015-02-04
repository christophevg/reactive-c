#ifndef __REACTIVE_H
#define __REACTIVE_H

#include "varargs.h"

// we're only exposing the observable type, the inner workings are private
typedef struct observable *observable_t;

// and a (linked) list for observables. use all() & co to create the list.
typedef struct observables *observables_t;

// an observer takes an array of arguments and a pointer to store its result
typedef void (*observer_t)(void**, void*);

// retrieve the current value of the observable
void *observable_value(observable_t);

// turns a variadic list of observables into a linked list of observables
observables_t __all(int,...);  // this signature is used _after_ macro expansion

// some utility macro's for various combinations of observables
#define all(...)   __all(_NARG(__VA_ARGS__), __VA_ARGS__)
#define just(x)    __all(1, x)
#define both(x, y) __all(2, x, y)

// adds observer to a list of observables, providing memory space for its value,
// based on its size
observable_t __observing(observables_t, observer_t, int);

// construct an observable from a (pointer to a) value
observable_t __observing_value(void*);

// allow observe() being called with optional arguments
// via: http://stackoverflow.com/questions/11761703/
#define __o1(v)       __observing_value((void*)&v)
#define __o2(l,o)     __observing(l,o,0)
#define __o3(l,o,t)   __observing(l,o,sizeof(t))
#define __o4(l,o,t,s) __observing(l,o,sizeof(t)*s)
#define __ox(_1,_2,_3,_4,NAME,...) NAME
#define observing(...) __ox(__VA_ARGS__, __o4, __o3, __o2, __o1)(__VA_ARGS__)

// acrually starts and observable
observable_t start(observable_t);

// helper macro to combine creation and activation
#define observe(...) start(observing(__VA_ARGS__))

// add a callback to the observable, triggered when it is disposed
typedef void (*observable_callback_t)(observable_t);
observable_t on_dispose(observable_t, observable_callback_t);
observable_t on_activation(observable_t, observable_callback_t);

// remove an observer from all observeds and releases it entirely
void dispose(observable_t);

// trigger and observable to be updated
void observe_update(observable_t observable);

// merge multiple observables, resulting in a single observable with interleaved
// updates
observable_t __merge(observables_t);
#define merge(...) __merge(all(__VA_ARGS__))

// maps an observable to something else ...
#define __m3(o,f,t)   observe(just(o),f,t)
#define __m4(o,f,t,s) observe(just(o),f,t,s)
#define __mx(_1, _2, _3, _4,NAME,...) NAME
#define map(...) __mx(__VA_ARGS__, __m4, __m3)(__VA_ARGS__)

// demo for lifting through macro-expansion
#define lift2(type, fun) \
  void __lifted_##fun(void **in, void *out) { \
    *(type*)(out) = fun((*(type*)(in[0])), (*(type*)(in[1]))); \
  }
#define lifted(x) __lifted_##x

// support for scripting

// script constructor takes a variable amount of (inactive) observables
observable_t __script(int, ...);
observable_t script(observable_t, ...);
#define script(...) __script(_NARG(__VA_ARGS__), __VA_ARGS__)

// explictly start a script
observable_t run(observable_t);

// morphing constructor to wait until an observable emits an update
observable_t await(observable_t);

#endif
