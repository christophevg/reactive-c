// an implementation of a futures/promises example from 
// https://github.com/cujojs/when

#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/api.h"

int sum(int a, int b) { return a + b;  }         lift_fold1(int, sum)
int times10(int a)    { return a * 10; }         lift1(int, times10)

void console_log(observable_t ob) {
  capture_printf("%d", *(int*)observable_value(ob));
}

int main(void) {
  observable_t remote_number_list = observed(int);

  on_dispose(
    reduce(map(remote_number_list, lifted(times10), int), lifted(sum), int),
    console_log
  );

  // simulate 5 integers arriving
  set(remote_number_list, 1);
  set(remote_number_list, 2);
  set(remote_number_list, 3);
  set(remote_number_list, 4);
  set(remote_number_list, 5);
  dispose(remote_number_list);

  assert_output_was("150");

  exit(EXIT_SUCCESS);
}
