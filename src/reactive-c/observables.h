#ifndef __OBSERVABLES_H
#define __OBSERVABLES_H

#include <stdbool.h>

#include "varargs.h"
#include "observable.h"

// and a (linked) list for observables. use each() & co to create the list.
typedef struct observables *observables_t;

// turns a variadic list of observables into a linked list of observables
observables_t __each(int,...);

// actual public API
#define each(...)  __each(_NARG(__VA_ARGS__), __VA_ARGS__)
#define just(x)    __each(1, x)
#define both(x, y) __each(2, x, y)

observables_t observables_new(void);
void observables_add(observables_t, observable_t);
void observables_remove(observables_t, observable_t);
void observables_clear(observables_t);
  
bool observables_contains(observables_t, observable_t);
bool observables_is_empty(observables_t list);

int observables_count(observables_t);

#endif
