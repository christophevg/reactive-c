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
#define __o1(v)       __observing_value(#v,(void*)&v)
#define __o2(l,o)     __observing(#l,l,o,0)
#define __o3(l,o,t)   __observing(#l,l,o,sizeof(t))
#define __o4(l,o,t,s) __observing(#l,l,o,sizeof(t)*s)
#define __ox(_1,_2,_3,_4,NAME,...) NAME
#define observing(...) __ox(__VA_ARGS__, __o4, __o3, __o2, __o1)(__VA_ARGS__)

// helper macro to combine creation and activation
#define observe(...) start(observing(__VA_ARGS__))

// function call with extended varargs
#define merge(...) __merge("merge(" #__VA_ARGS__ ")", each(__VA_ARGS__))
#define all(...)   __all("all(" #__VA_ARGS__ ")", each(__VA_ARGS__))
#define any(...)   __any("any(" #__VA_ARGS__ ")", each(__VA_ARGS__))

// overloaded function and extended varargs
#define __m3(o,f,t)   observe(just(o),f,t)
#define __m4(o,f,t,s) observe(just(o),f,t,s)
#define __mx(_1, _2, _3, _4,NAME,...) NAME
#define map(...) __mx(__VA_ARGS__, __m4, __m3)(__VA_ARGS__)

// demo for lifting through macro-expansion
#define lift2(type, fun) \
  void __lifted_##fun(observation_t ob) { \
    *(type*)(ob->observer) = fun((*(type*)(ob->observeds[0])), (*(type*)(ob->observeds[1]))); \
  }
#define lifted(x) __lifted_##x

// constructor with extended varargs
#define script(...) __script(_NARG(__VA_ARGS__), __VA_ARGS__)

// sometimes its nicer to write... e.g. return suspended(this)
#define suspended(x) suspend(x)
#define started(x)   start(x)
#define delayed(x)   delay(x)

// output support
#define __d1(x)    __to_dot(x, stdout, true)
#define __d2(x, f) __to_dot(x, f, true)
#define __dx(_1,_2,NAME,...) NAME
#define to_dot(...) __dx(__VA_ARGS__, __d2, __d1)(__VA_ARGS__)
