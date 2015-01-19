#include "reactive.h"
#include "temp_behavior.h"

double temp = 123;

void temp_behavior_init() {
  temp_behavior = observable_from_value((void*)&temp);
}

// function to allow simulation of updates to temp behavior
void temp_update(double update) {
  temp = update;
  // trigger update propagation
  observe_update(temp_behavior);
}
