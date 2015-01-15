// clock observable

#include <stdio.h>

#include "clock_observable.h"

// we use a virtual clock to perform the simulation

int clock;

void clock_init(void) {
  clock = 0;
  clock_observable = observable_new();
}

// we provide a function to simulate clock activity
void clock_tick(int ms) {
  clock += ms;
  printf("clock : increasing time by %d to %d\n", ms, clock);
  observable_notify(clock_observable, NULL);
}

int clock_now(void) {
  return clock;
}