#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/reactive.h"

// helper functionality to track steps of the script
int steps = 0;
void count_steps(observable_t _) { steps++; }
void add_step_counter(observable_t observable) {
  on_dispose(observable, count_steps);
}

int main(void) {
  int _a = 0, _b = 0;
  
  observable_t a = observe(_a);
  observable_t b = observe(_b);
  
  run(
    on_activation(
      script(
        await(a),
        await(b),
        await(a)
      ),
      add_step_counter
    )
  );

  assert_zero(steps, "Expected no steps, observed %d.\n", steps);

  _b = 1; observe_update(b);  // does nothing

  assert_zero(steps, "Expected no steps, observed %d.\n", steps);

  _a = 1; observe_update(a);  // finalizes await(a)

  assert_equal(steps, 1, "Expected 1 step, observed %d.\n", steps);

  _a = 2; observe_update(a);  // does nothing

  assert_equal(steps, 1, "Expected 1 step, observed %d.\n", steps);

  _b = 2; observe_update(b);  // finalizes await(b)

  assert_equal(steps, 2, "Expected 2 steps, observed %d.\n", steps);

  _b = 3; observe_update(b);  // does nothing

  assert_equal(steps, 2, "Expected 2 steps, observed %d.\n", steps);

  _a = 3; observe_update(a);  // finalizes await(a)

  assert_equal(steps, 3, "Expected 3 steps, observed %d.\n", steps);

  _a = 4; observe_update(a);  // does nothing

  assert_equal(steps, 3, "Expected 3 steps, observed %d.\n", steps);

  _b = 4; observe_update(b);  // does nothing

  assert_equal(steps, 3, "Expected 3 steps, observed %d.\n", steps);

  exit(EXIT_SUCCESS);
}
