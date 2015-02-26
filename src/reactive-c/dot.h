#ifndef __DOT_H
#define __DOT_H

#include <stdbool.h>
#include <stdio.h>

#include "observable.h"

// dumps the given observable (recursively) in dot format
void __to_dot(observable_t, FILE*, bool, bool);

#define __d1(x)       __to_dot(x, stdout, false, true)
#define __d2(x, f)    __to_dot(x, f,      false, true)
#define __d3(x, f, m) __to_dot(x, f,      m,     true)

#define __dx(_1, _2, _3, NAME,...) NAME

// actual public API
#define to_dot(...) __dx(__VA_ARGS__, __d3, __d2, __d1)(__VA_ARGS__)

#endif
