#include <stdlib.h>
#include <string.h>

#include "unknown.h"

#include "../unit/test.h"

void display(unknown_t data) {
  capture_printf("%d = %f = %s\n",
                 as(int,    data),
                 as(double, data),
                 as(string, data));
}

int main(void) {
  int i = 123;
  unknown_t value1 = new(int, i);
  display(value1);

  assert_output_was("123 = 123.000000 = 123\n"); clear_output();
  
  double d = 123.123;
  unknown_t value2 = new(double, d);
  display(value2);

  assert_output_was("123 = 123.123000 = 123.123000\n"); clear_output();

  char *s = "123.123";
  unknown_t value3 = new(string, s);
  display(value3);

  assert_output_was("123 = 123.123000 = 123.123\n"); clear_output();

  // let support

  double d2 = 456.789;
  let(value1, new(double, d2));

  assert_equal(as(int, value1), 457, "Expected 456.789 as int to be 457.\n");

  let(value1, double, 123.123);

  assert_equal(as(int, value1), 123, "Expected 123.123 as int to be 123.\n");

  val(int, value1) = (int)987.123;

  assert_equal(as(int, value1), 987, "Expected 987 as int to be 987 .\n");

  char *s2 =  "hello world";
  unknown_t str = new(string, s2);
  val(string, str) = "updated world";

  assert_true(strcmp(as(string, str), "updated world") == 0,
              "Expected updated string");

  assert_true(is(int, value1), "Expected value1 to be an integer.");
  assert_false(is(double, value1), "Expected value1 NOT to be a double.");
  assert_true(is(string, str), "Expected str to be a string.");

  exit(EXIT_SUCCESS);
}
