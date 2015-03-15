#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/api.h"

int main(void) {
  int _a = 0, _b = 0;
  
  observable_t a = observe(int, _a);
  observable_t b = observe(int, _b);

  observable_t s = 
    script(
      await(a),
      b
    );

  run(s);

  // await(a)
  assert_true(observable_value(s) == NULL, "Expected NULL.");

  set(int, b, 123);  // does nothing
  assert_true(observable_value(s) == NULL, "Expected NULL.");

  set(int, b, 456);  // does nothing
  assert_true(observable_value(s) == NULL, "Expected NULL.");

  set(int, a, 123);  // finalizes await(a)
  assert_true(as(int, observable_value(s)) == _b, "Expected == _b.");

  exit(EXIT_SUCCESS);
}
