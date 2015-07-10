// demo for "add"

#include <stdlib.h>
#include "reactive-c/api.h"

// too bad we can't do operator overloading in C ;-(
int add(int a, int b) {  return a + b; }

// (basic) explicit lifting of non-lifted function ;-(
lift2(int, add)

int main(void) {
  // the primitive values
  int _a, _b, _c;
  
  // the observing counterparts
  observable_t a = observe(_a);
  observable_t b = observe(_b);
  observable_t c = observe(_c);
  
  // a = b + c
  let(a, be(add, int, b, c));
  
  // too bad we can't do operator overloading in C ;-(
  _b = 10;
  _c = 42;
  
  // trigger observation manually ;-(
  observe_update(c);
  
  printf("_a = _b + _c = %d + %d = %d\n", _b, _c, _a);

  exit(EXIT_SUCCESS);
}
