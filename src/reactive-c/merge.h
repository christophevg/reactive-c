#ifndef __MERGE_H
#define __MERGE_H

#include "observable.h"
#include "observables.h"

// create a single observable observer from a list of observed observables.
observable_t __merge(char*, observables_t);

// actual public API
#define merge(...) __merge("merge(" #__VA_ARGS__ ")", each(__VA_ARGS__))

#endif
