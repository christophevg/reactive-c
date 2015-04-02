#ifndef __DOT_H
#define __DOT_H

#include <stdbool.h>
#include <stdio.h>

#include "observable.h"

// dumps the given observable (recursively) in dot format
void __to_dot(observable_t, FILE*, int, bool);

#define DOT_DEFAULT          0
#define DOT_SHOW_MEMORY      1
#define DOT_MESSAGE_STYLE    2
#define DOT_HIDE_LABELS      4
#define DOT_SMALL_RANK_SEP   8
#define DOT_SHAPE_IS_CIRCLE 16
#define DOT_SHOW_MARKED     32
#define DOT_SHOW_LEVEL      64

#define __d1(x)          __to_dot(x, stdout, DOT_DEFAULT, true)
#define __d2(x, f)       __to_dot(x, f,      DOT_DEFAULT, true)
#define __d3(x, f, s)    __to_dot(x, f,      s,           true)

#define __dx(_1, _2, _3, NAME,...) NAME

// actual public API
#define to_dot(...) __dx(__VA_ARGS__, __d3, __d2, __d1, 0)(__VA_ARGS__)

#endif
