// demo for map

#include <stdlib.h>
#include <stdio.h>

#include "reactive.h"

void display(void **args, void *_) {
  printf("current value = value: %s.\n", (char*)(args[0]));
}

// converts double to string representation
void convert(void **number, void *string) {
  snprintf(((char*)string), 10, "%f", *(double*)(number[0]));
}

int main(void) {
  double _a;
  
  // observer a value
  observable_t a = observable_from_value((void*)&_a);

  // map the observed value to something else
  observable_t A = map(a, convert, sizeof(char)*10);
  // which is basically a wrapper for...
  // observable_t A = observe(all(a), convert, sizeof(char)*10);

  observe(all(A), display, 0);

  _a = 1;  observe_update(a);
  _a = 2;  observe_update(a);
  _a = 3;  observe_update(a);

  exit(EXIT_SUCCESS);
}
