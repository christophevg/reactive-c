// demo for "filter"

#include <stdlib.h>
#include <stdio.h>

#include "unit/test.h"

#include "reactive-c/api.h"

bool odd(unknown_t value) {
  return *(int*)value % 2 != 0;
}

int main(void) {
  int _var1;
  observable_t var1 = observe(_var1);

  observable_t filtered = filter(int, var1, odd);

  set(var1, 1);

  assert_equal(*(int*)observable_value(var1),     1, "Expected value = 1\n");
  assert_equal(*(int*)observable_value(filtered), 1, "Expected value = 1\n");
  
  set(var1, 2);

  assert_equal(*(int*)observable_value(var1),     2, "Expected value = 2\n");
  assert_equal(*(int*)observable_value(filtered), 1, "Expected value = 1\n");

  set(var1, 3);

  assert_equal(*(int*)observable_value(var1),     3, "Expected value = 3\n");
  assert_equal(*(int*)observable_value(filtered), 3, "Expected value = 3\n");

  exit(EXIT_SUCCESS);
}
