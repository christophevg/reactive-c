#ifndef __FOLDING_H
#define __FOLDING_H

observable_t __fold_int(observable_t, observer_t, int);
observable_t __fold_double(observable_t, observer_t, double);

#define __f3(o,f,t)   observe(just(o),f,t)
#define __f4(o,f,t,i) __fold_##t(o,f,i)

#define __fx(_1,_2,_3,_4,NAME,...) NAME

// actual public API
#define fold(...) __fx(__VA_ARGS__, __f4, __f3)(__VA_ARGS__)

#endif
