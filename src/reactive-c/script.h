#ifndef __SCRIPT_H
#define __SCRIPT_H

#include "observable.h"

// script constructor takes a variable amount of (inactive) observables
observable_t __script(int, ...);

// explictly start a script
observable_t run(observable_t);

// morphing constructor to wait until an observable emits an update
observable_t await(observable_t);

// actual public API
#define script(...) __script(_NARG(__VA_ARGS__), __VA_ARGS__)

#endif
