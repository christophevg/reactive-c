// demo for map

#include <stdlib.h>
#include <stdio.h>

#include "unit/test.h"

#include "reactive-c/api.h"

void display(observation_t ob) {
  capture_printf("current value = value: %s.\n", (char*)(ob->observeds[0]));
}

// converts double to string representation
void double2string(observation_t ob) {
  snprintf(((char*)ob->observer), 10, "%0.f", *(double*)(ob->observeds[0]));
}

int main(void) {
  double _a;
  
  // observer a value
  observable_t a = observe(int, _a);

  // map the observed value to something else
  observable_t A = map(a, double2string, char, 10);
  // which is basically a wrapper for...
  // observable_t A = observe(just(a), double2string, char, 10);

  observe(just(A), display, void*);

  _a = 1;  observe_update(a);
  _a = 2;  observe_update(a);
  _a = 3;  observe_update(a);

  assert_output_was(
    "current value = value: 1.\n"
    "current value = value: 2.\n"
    "current value = value: 3.\n"
  );

  exit(EXIT_SUCCESS);
}
