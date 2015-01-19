#ifndef __TEMP_BEHAVIOR_H
#define __TEMP_BEHAVIOR_H

#include "reactive.h"

observable_t temp_behavior;

void temp_behavior_init(void);

// helper function for simulation purposes
void temp_update(double);

#endif
