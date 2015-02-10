#include <stdlib.h>
#include <string.h>

#include "unknown/unknown.h"

#include "unit/test.h"

void display(unknown_t data) {
  capture_printf("%d = %f = %s\n",
                 as(int,    data),
                 as(double, data),
                 as(string, data));
}

int main(void) {
  unknown_t value1 = new(int, 123);
  display(value1);

  assert_output_was("123 = 123.000000 = 123\n"); clear_output();
  
  unknown_t value2 = new(double, 123.123);
  display(value2);

  assert_output_was("123 = 123.123000 = 123.123000\n"); clear_output();

  unknown_t value3 = new(string, "123.123");
  display(value3);

  assert_output_was("123 = 123.123000 = 123.123\n"); clear_output();

  // let support

  let(value1, new(double, 456.789));

  assert_equal(as(int, value1), 457, "Expected 456.789 as int to be 457.\n");

  let(value1, double, 123.123);

  assert_equal(as(int, value1), 123, "Expected 123.123 as int to be 123.\n");

  val(int, value1) = (int)987.123;

  assert_equal(as(int, value1), 987, "Expected 987 as int to be 987 .\n");

  unknown_t str = new(string, "hello world");
  val(string, str) = "updated world";

  assert_true(strcmp(as(string, str), "updated world") == 0,
              "Expected updated string");

  exit(EXIT_SUCCESS);
}
