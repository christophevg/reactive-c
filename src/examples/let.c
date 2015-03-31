#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/api.h"

int main(void) {
  int _a = 0, _b = 0, _c = 0;
  
  observable_t a = observe(_a);
  observable_t b = observe(_b);
  observable_t c = observe(_c);

  // we might want to do...
  // observable_t a = merge(b,c);
  // but... observable a already exists, so we want to change it.
  // let copies all info from one observable to the other

  let(a, merge(b, c));

  set(a, 1);
  
  assert_equal(_a, 1, "a should be 1\n");
  assert_equal(_b, 0, "b should be 0\n");
  assert_equal(_c, 0, "c should be 0\n");
  
  set(b, 2);

  assert_equal(_a, 2, "a should be 2\n");
  assert_equal(_b, 2, "b should be 2\n");
  assert_equal(_c, 0, "c should be 0\n");

  set(c, 3);
  
  assert_equal(_a, 3, "a should be 3\n");
  assert_equal(_b, 2, "b should be 2\n");
  assert_equal(_c, 3, "c should be 3\n");

  exit(EXIT_SUCCESS);
}
