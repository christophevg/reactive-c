#ifndef __FOLDING_H
#define __FOLDING_H

observable_t __fold_int(observable_t, observer_t, int);
observable_t __fold_double(observable_t, observer_t, double);

#define __f3(o,f,t)   observe(just(o),f,t)
#define __f4(o,f,t,i) __fold_##t(o,f,i)

#define __fx(_1,_2,_3,_4,NAME,...) NAME

// actual public API
#define fold(...) __fx(__VA_ARGS__, __f4, __f3, __f2, __f1, 0)(__VA_ARGS__)

// reduce is actually a fold, which takes the first value as its initialization
// in case that is behaves the same as starting with 0, we can reuse the default
// fold implementation

#define reduce(o,f,t) auto_dispose(fold(o,f,t))

#endif
