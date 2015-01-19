// implementation of the temperature conversion behavior from the reactive
// programming survey ...

// we have a behavior temp that continuously presents the current temperature
// in degrees Celcius

#include <stdlib.h>
#include <stdio.h>

#include "reactive.h"

#include "temp_behavior.h"

// a user defined convertion between Celcius and Farenheit
void c2f(void *c, void *f) {
  (*(double*)(f)) = ( (*(double*)(c)) * 1.8 ) + 32;
}

// a user defined display function
void display(void *value, void *_) {
  printf( "observable was updated to %f\n", *(double*)(value) );
}

int main(void) {
  // init temp behavior
  temp_behavior_init();
  
  // create a new behviour that convers C to F
  observable_t temp_f = observe(temp_behavior, c2f, sizeof(double));

  // simulate changes to temp
  temp_update(16);
  temp_update(17);
  temp_update(18);
  
  // so far no output
  
  // some debugging feedback
  printf("current temp=%f | temp_f=%f\n", *(double*)observable_value(temp_behavior),
                                          *(double*)observable_value(temp_f));

  // let's add an observer that displays the updates

  // display the behavior
  observe(temp_f, display, 0);

  temp_update(19);
  temp_update(20);
  temp_update(21);
  
  exit(EXIT_SUCCESS);
}
