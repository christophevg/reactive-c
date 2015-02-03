// demo for merging of observables

#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/reactive.h"

void display(void **args, void* _) {
  capture_printf("current value = value: %f\n", *(double*)(args[0]));
}

int main(void) {
  double _a, _b, _c;
  
  // create three basic observers
  observable_t a = observe(_a);
  observable_t b = observe(_b);
  observable_t c = observe(_c);

  // create one that merges all three
  observable_t abc = merge(a, b, c);

  // observe all (three) updates in one go
  observe(just(abc), display);

  _a = 1;  observe_update(a);
  _b = 2;  observe_update(b);
  _c = 3;  observe_update(c);

  assert_output_was(
    "current value = value: 1.000000\n"
    "current value = value: 2.000000\n"
    "current value = value: 3.000000\n"
  );

  exit(EXIT_SUCCESS);
}
