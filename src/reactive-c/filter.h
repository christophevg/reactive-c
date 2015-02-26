#ifndef __FILTER_H
#define __FILTER_H

#include <stdbool.h>

#include "observable.h"

observable_t __filter(int size, observable_t, validator_t);

// actual public API
#define filter(t,o,v) __filter(sizeof(t),o,v)

#endif
