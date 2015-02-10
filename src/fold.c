// demo for "fold"

#include <stdlib.h>
#include <stdio.h>

#include "unit/test.h"

#include "reactive-c/reactive.h"

void fold_sum(observation_t ob) {
  *((int*)ob->observer) += *((int*)ob->observeds[0]);
}

int main(void) {
  int _var1;
  observable_t var1 = observe(int, _var1);

  observable_t folded = fold(var1, fold_sum, int, 3);

  _var1 = 1;  observe_update(var1);

  assert_equal(*(int*)observable_value(folded), 4, "Expected value = 4\n");
  
  _var1 = 2;  observe_update(var1);

  assert_equal(*(int*)observable_value(folded), 6, "Expected value = 6\n");

  _var1 = 3;  observe_update(var1);

  assert_equal(*(int*)observable_value(folded), 9, "Expected value = 9\n");

  exit(EXIT_SUCCESS);
}
