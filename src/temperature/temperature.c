// implementation of the temperature conversion behavior from the reactive
// programming survey ...

// we have a behavior temp that continuously presents the current temperature
// in degrees Celcius

#include <stdlib.h>
#include <stdio.h>

#include "reactive.h"

#include "temp_behavior.h"

// a user defined convertion between Celcius and Farenheit
void c2f(void **args, void *f) {
  (*(double*)(f)) = ( (*(double*)(args[0])) * 1.8 ) + 32;
}

// a user defined display function to display both C and F values
void display(void **args, void *_) {
  printf( "observable was updated to %fC/%fF\n",
          *(double*)(args[0]), *(double*)(args[1]) );
}

// temp_behavior 0    <-- update
//   /     \
// temp_f   |    1
//   \     /
//   display     2

int main(void) {
  // init temp behavior
  temp_behavior_init();

  // create a new behviour that convers C to F
  observable_t temp_f = observe(just(temp_behavior), c2f, sizeof(double));

  // simulate changes to temp
  temp_update(16);
  temp_update(17);
  temp_update(18);
  
  // so far no output
  
  // some debugging feedback
  printf( "current temp=%f | temp_f=%f\n",
          *(double*)observable_value(temp_behavior),
          *(double*)observable_value(temp_f));

  // let's add an observer that displays the updates

  // display updates from now on using our display observer function
  observable_t displayer = observe(all(temp_behavior, temp_f), display, 0);

  temp_update(19);
  temp_update(20);
  temp_update(21);
  
  dispose(displayer);

  temp_update(22);
  temp_update(23);
  temp_update(24);

  // some debugging feedback
  printf( "current temp=%f | temp_f=%f\n",
          *(double*)observable_value(temp_behavior),
          *(double*)observable_value(temp_f));

  dispose(temp_f);

  exit(EXIT_SUCCESS);
}
