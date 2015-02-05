// macro expansions for some syntactic sugar

// two major aspects are implemented this way:
// 1. variable argument lists are augmented with their count
// 2. overloaded functions, based on number of supplied arguments
// concepts learned from:
// - http://stackoverflow.com/questions/11761703/

#include "varargs.h"

// utility function wrappers for __each(int, ...)
#define each(...)  __each(_NARG(__VA_ARGS__), __VA_ARGS__)
#define just(x)    __each(1, x)
#define both(x, y) __each(2, x, y)

// overloaded constructor for creating observables
#define __o1(v)       __observing_value((void*)&v)
#define __o2(l,o)     __observing(l,o,0)
#define __o3(l,o,t)   __observing(l,o,sizeof(t))
#define __o4(l,o,t,s) __observing(l,o,sizeof(t)*s)
#define __ox(_1,_2,_3,_4,NAME,...) NAME
#define observing(...) __ox(__VA_ARGS__, __o4, __o3, __o2, __o1)(__VA_ARGS__)

// helper macro to combine creation and activation
#define observe(...) start(observing(__VA_ARGS__))

// function call with extended varargs
#define merge(...) __merge(each(__VA_ARGS__))

// overloaded function and extended varargs
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

// constructor with extended varargs
#define script(...) __script(_NARG(__VA_ARGS__), __VA_ARGS__)
