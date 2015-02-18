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
  int _a = 0, _b = 0, _c = 0;
  
  observable_t a = observe(int, _a);
  observable_t b = observe(int, _b);
  observable_t c = observe(int, _c);

  observable_t s = 
    script(
        await(a),
        await(b),
        await(delayed(all(a, b, c))),
        await(delayed(any(b, c))),
        await(        all(a, b, c))
    );

  run( on_activation(s, add_step_counter) );

  // await(a)
  assert_zero(steps, "Expected no steps, observed %d.\n", steps);

  _b = 1; observe_update(b);  // does nothing
  assert_zero(steps, "Expected no steps, observed %d.\n", steps);

  _a = 1; observe_update(a);  // finalizes await(a)
  assert_equal(steps, 1, "Expected 1 step, observed %d.\n", steps);

  // await(b)
  _a = 2; observe_update(a);  // does nothing
  assert_equal(steps, 1, "Expected 1 step, observed %d.\n", steps);

  _b = 2; observe_update(b);  // finalizes await(b)
  assert_equal(steps, 2, "Expected 2 steps, observed %d.\n", steps);

  // await(delayed(all(a,b,c)))
  _b = 3; observe_update(b);  // does nothing
  assert_equal(steps, 2, "Expected 2 steps, observed %d.\n", steps);

  _a = 3; observe_update(a);  // does nothing
  assert_equal(steps, 2, "Expected 2 steps, observed %d.\n", steps);

  _c = 3; observe_update(c);  // finalizes await(delayed(all(a,b,c)))
                              // but also the already started all(a,b,c)
                              // but not its await step
  assert_equal(steps, 3, "Expected 3 steps, observed %d.\n", steps);

  // await(delayed(any(b,c)))
  _a = 4; observe_update(a);  // does nothing
  assert_equal(steps, 3, "Expected 3 steps, observed %d.\n", steps);

  _c = 4; observe_update(c);  // finalizes await(delayed(any(b,c)))
                              // this also moves the script forward to the last
                              // await(all(a,b,c)), of which the all()
                              // observable has already finished
  assert_equal(steps, 5, "Expected 5 steps, observed %d.\n", steps);

  // script done, nothing triggered anymore
  _a = 5; observe_update(a);  // does nothing
  assert_equal(steps, 5, "Expected 5 steps, observed %d.\n", steps);

  _b = 5; observe_update(b);  // does nothing
  assert_equal(steps, 5, "Expected 5 steps, observed %d.\n", steps);

  _c = 5; observe_update(c);  // does nothing
  assert_equal(steps, 5, "Expected 5 steps, observed %d.\n", steps);

  // clean up to validate memory leaks ;-)
  dispose(s);
  dispose(a);
  dispose(b);
  dispose(c);

  empty_trash();

  exit(EXIT_SUCCESS);
}
