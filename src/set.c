#include <stdio.h>
#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/reactive.h"

int main(void) {
  int    _a = 0;
  double _b = 0;
  
  observable_t a = observe(int,    _a);
  observable_t b = observe(double, _b);

  assert_equal(_a, 0, "a should be 0\n");
  assert_equal(_b, 0, "b should be 0\n");

  set(int, a, 3);

  assert_equal(_a, 3, "a should be 3\n");
  assert_equal(_b, 0, "b should be 0\n");

  set(double, b, 3);
    
  assert_equal(_a, 3, "a should be 3\n");
  assert_equal(_b, 3, "b should be 3\n");

  exit(EXIT_SUCCESS);
}
