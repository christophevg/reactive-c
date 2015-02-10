// demo of "lifting"

#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/reactive.h"

// "a" function taking two integers and returning an int
int add(int a, int b) {
  return a + b;
}

lift2(int, add);

// expands to...
// void lifted_add(observation_t ob) {
//   *(int*)(ob->observer) =
//     add((*(int*)(ob->observeds[0])), (*(int*)(ob->observeds[1])));
// }

int main(void) {
  int _var1, _var2;
  
  // create two observable values
  observable_t var1 = observe(int, _var1);
  observable_t var2 = observe(int, _var2);
  
  // and one observable combination (aka behavior)
  observable_t var3 = observe(both(var1, var2), lifted(add), int);

  // simulate some events on all
  _var1 = 1;  observe_update(var1);
  _var2 = 2;  observe_update(var2);

  // at this point var3 is already updated, let's check that:

  capture_printf("%d + %d = %d\n", _var1, _var2, *(int*)observable_value(var3));

  assert_output_was( "1 + 2 = 3\n" );
  clear_output();

  _var1 = 3;  observe_update(var1);
  _var2 = 5;  observe_update(var2);

  // at this point var3 is already (again) updated, let's check that (again):

  capture_printf("%d + %d = %d\n", _var1, _var2, *(int*)observable_value(var3));

  assert_output_was( "3 + 5 = 8\n" );

  exit(EXIT_SUCCESS);
}
