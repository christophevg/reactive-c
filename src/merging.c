// demo for merging of observables

#include <stdio.h>
#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/reactive.h"

void display(observation_t ob) {
  capture_printf("current value = value: %f\n", *(double*)(ob->observeds[0]));
}

int main(void) {
  double _a, _b, _c;
  
  // create three basic observers
  observable_t a = observe(double, _a);
  observable_t b = observe(double, _b);
  observable_t c = observe(double, _c);

  // create one that merges all three
  observable_t abc = merge(a, b, c);

  // observe all (three) updates in one go
  observe(just(abc), display, void);

  _a = 1;  observe_update(a);

  assert_output_was(
    "current value = value: 1.000000\n"
  );

  _b = 2;  observe_update(b);

  assert_output_was(
    "current value = value: 1.000000\n"
    "current value = value: 2.000000\n"
  );

  _c = 3;  observe_update(c);

  assert_output_was(
    "current value = value: 1.000000\n"
    "current value = value: 2.000000\n"
    "current value = value: 3.000000\n"
  );

  exit(EXIT_SUCCESS);
}
