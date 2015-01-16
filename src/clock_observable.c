// clock observable

#include "clock_observable.h"

// we use a virtual clock to perform the simulation

int clock;

void clock_init(void) {
  clock = 0;
  clock_observable = observable_new();
}

void clock_tick(int ms) {
  clock += ms;
  observable_notify(clock_observable, NULL);
}

int clock_now(void) {
  return clock;
}
