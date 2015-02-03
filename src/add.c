// demo for "add"

#include <stdlib.h>
#include <stdio.h>

#include "reactive-c/reactive.h"

int main(void) {
  int _var1, _var2;
  
  // create two observable values
  observable_t var1 = observe(_var1);
  observable_t var2 = observe(_var2);
  
  // and one observable combination
  observable_t var3 = addi(var1, var2);

  // simulate some events on all
  _var1 = 1;  observe_update(var1);
  _var2 = 2;  observe_update(var2);

  // at this point var3 is already updated, let's check that:

  printf("%d + %d = %d\n", _var1, _var2, *(int*)observable_value(var3));

  _var1 = 3;  observe_update(var1);
  _var2 = 5;  observe_update(var2);

  // at this point var3 is already (again) updated, let's check that (again):

  printf("%d + %d = %d\n", _var1, _var2, *(int*)observable_value(var3));

  exit(EXIT_SUCCESS);
}
