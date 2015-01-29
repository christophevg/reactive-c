// demo for merging of observables

#include <stdlib.h>
#include <stdio.h>

#include "reactive.h"

void display(void **args, void* _) {
  printf("current value = value: %f\n", *(double*)(args[0]));
}

int main(void) {
  double _a, _b, _c;
  
  // create three basic observers
  observable_t a = observable_from_value((void*)&_a);
  observable_t b = observable_from_value((void*)&_b);
  observable_t c = observable_from_value((void*)&_c);

  // create one that merges all three
  observable_t abc = merge(all(a, b, c));

  // observe all (three) updates in one go
  observe(all(abc), display, 0);

  _a = 1;  observe_update(a);
  _b = 2;  observe_update(b);
  _c = 3;  observe_update(c);

  exit(EXIT_SUCCESS);
}
