#include <stdio.h>
#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/api.h"

int main(void) {
  int _a = 0, _b = 0;
  
  observable_t a = observe(_a);
  observable_t b = observe(_b);

  _a = 3;
  
  assert_equal(_a, 3, "a should be 3\n");
  assert_equal(_b, 0, "b should be 0\n");

  observable_value_copy(a, b);

  assert_equal(_a, 3, "a should be 3\n");
  assert_equal(_b, 3, "b should be 3\n");

  exit(EXIT_SUCCESS);
}
