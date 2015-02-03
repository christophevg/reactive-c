#ifndef __REACTIVE_H
#define __REACTIVE_H

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

// marco's to allow all(...) to be expanded to __all(int,...)
// via: https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s
#define PP_NARG(...) \
        PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
        PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
        _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
        63,62,61,60,                   \
        59,58,57,56,55,54,53,52,51,50, \
        49,48,47,46,45,44,43,42,41,40, \
        39,38,37,36,35,34,33,32,31,30, \
        29,28,27,26,25,24,23,22,21,20, \
        19,18,17,16,15,14,13,12,11,10, \
         9, 8, 7, 6, 5, 4, 3, 2, 1, 0

// some utility macro's for various combinations of observables
#define all(...)   __all(PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define just(x)    __all(1, x)
#define both(x, y) __all(2, x, y)

// adds observers to a list of observables, providing memory space for its
// value, based on its size
observable_t __observe(observables_t, observer_t, int);

// construct an observable from a (pointer to a) value
observable_t __observe_value(void*);

// allow observe() being called with optional arguments
// via: http://stackoverflow.com/questions/11761703/
#define __observe1(v)       __observe_value((void*)&v)
#define __observe2(l,o)     __observe(l,o,0)
#define __observe3(l,o,t)   __observe(l,o,sizeof(t))
#define __observe4(l,o,t,s) __observe(l,o,sizeof(t)*s)
#define GET_OBSERVE(_1,_2,_3,_4,NAME,...) NAME
#define observe(...) GET_OBSERVE(__VA_ARGS__, __observe4, __observe3, __observe2, __observe1)(__VA_ARGS__)

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
#define __map3(o,f,t)   observe(just(o),f,t)
#define __map4(o,f,t,s) observe(just(o),f,t,s)
#define GET_MAP(_1, _2, _3, _4,NAME,...) NAME
#define map(...) GET_MAP(__VA_ARGS__, __map4, __map3)(__VA_ARGS__)

observable_t addi(observable_t, observable_t);
observable_t addd(observable_t, observable_t);

// demo for lifting through macro-expansion
#define lift2(type, fun) \
  void __lifted_##fun(void **in, void *out) { \
    *(type*)(out) = fun((*(type*)(in[0])), (*(type*)(in[1]))); \
  }
#define lifted(x) __lifted_##x

// support for scripting

// a script consist of fragments
typedef struct fragment *fragment_t;

// script constructor takes a variable amount of fragments
observable_t __script(int, ...);
observable_t script(fragment_t, ...);
#define script(...) __script(PP_NARG(__VA_ARGS__), __VA_ARGS__)

// explictly start a script
observable_t run(observable_t);

// fragment constructor to wait until an observable emits an update
fragment_t await(observable_t);

#endif
