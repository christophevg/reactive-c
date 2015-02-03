// helper functions for basic unit testing

#ifndef __TEST_H
#define __TEST_H

#include <stdarg.h>
#include <stdbool.h>

// a replacement for printf to capture the output
void capture_printf(char *, ...);

// assertion function to allow more elaborated assertion error messages
void assert_true(bool, char *, ...);

#define assert_false(a,...)     assert_true(!a, __VA_ARGS__)
#define assert_zero(v,...)      assert_true(v == 0, __VA_ARGS__)
#define assert_equal(x, y, ...) assert_true(x == y, __VA_ARGS__)

void assert_no_output(void);
void assert_output(void);
void assert_output_was(char*);

void clear_output(void);

#endif
