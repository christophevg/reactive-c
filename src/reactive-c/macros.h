// macro expansions for some syntactic sugar

// two major aspects are implemented this way:
// 1. variable argument lists are augmented with their count
// 2. overloaded functions, based on number of supplied arguments
// concepts learned from:
// - http://stackoverflow.com/questions/11761703/
// - https://groups.google.com/forum/?fromgroups#!topic/comp.std.c/d-6Mj5Lko_s

#include "varargs.h"

// utility function wrappers for __each(int, ...)
#define each(...)  __each(_NARG(__VA_ARGS__), __VA_ARGS__)
#define just(x)    __each(1, x)
#define both(x, y) __each(2, x, y)

// overloaded constructor for creating observables
#define __o2(t,v)     __observing_value(#v,(void*)&v,sizeof(t))
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

// folding support
#define __f3(o,f,t)   observe(just(o),f,t)
#define __f4(o,f,t,i) __fold_##t(o,f,i)
#define __fx(_1,_2,_3,_4,NAME,...) NAME
#define fold(...) __fx(__VA_ARGS__, __f4, __f3)(__VA_ARGS__)

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
#define __d1(x)       __to_dot(x, stdout, false, true)
#define __d2(x, f)    __to_dot(x, f, false, true)
#define __d3(x, f, m) __to_dot(x, f, m, true)
#define __dx(_1,_2,_3,NAME,...) NAME
#define to_dot(...) __dx(__VA_ARGS__, __d3, __d2, __d1)(__VA_ARGS__)

// set support
#define set(t,o,v) __set_##t(o,v)

// filter support
#define filter(t,o,v) __filter(sizeof(t),o,v)