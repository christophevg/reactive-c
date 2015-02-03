// helper functions for basic unit testing

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "test.h"

char _stdout[4096] = "";

// a replacement for printf to capture the output
void capture_printf(char *format, ...) {
  va_list args;
  // create the formatted string
  char string[256] = "";
  va_start(args, format);
  vsnprintf(string, 256, format, args);
  va_end(args);
  // concatenate it to the internal stdout buffer
  strcat(_stdout, string);
}

// assertion function to allow more elaborated assertion error messages
void assert_true(bool asserted, char *message, ...) {
  if(asserted) { return; }
  va_list args;
  fprintf(stderr, "Assertion failed: ");
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
  abort();
}

// assertion functions

void assert_no_output(void) {
  assert_true(
    strcmp(_stdout,  "") == 0,
    "Output produced, while none expected:\n"
    "output =\n%s", _stdout
  );
}

void assert_output(void) {
  assert_true(
    strcmp(_stdout,  "") != 0,
    "No output produced, while expected."
  );
}

void assert_output_was(char* expected) {
  assert_true(
    (strlen(_stdout) == strlen(expected)) &&
    (strncmp(_stdout, expected, strlen(expected)) == 0),
    "Output differs from expected:\n"
    "actual =\n%s\nexpected =\n%s\n", _stdout, expected
  );
}

void clear_output(void) {
  _stdout[0] = '\0';
}
