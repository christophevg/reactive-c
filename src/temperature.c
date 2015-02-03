// implementation of the temperature conversion behavior from the reactive
// programming survey ...

// we have a behavior temp that continuously presents the current temperature
// in degrees Celcius

#include <stdlib.h>
#include <stdio.h>

#include "reactive-c/reactive.h"

// to observe a value/variable...
double temp = 123;

// ...we create an observable...
observable_t observable_temp;

// ... and link it to the value/variable.
void observable_temp_init() {
  observable_temp = observe(temp);
}

// when the value/variable is updated, we trigger the RP functionality
void temp_update(double update) {
  temp = update;
  // trigger update propagation
  observe_update(observable_temp);
}

// a user defined convertion between Celcius and Farenheit
void c2f(void **args, void *f) {
  (*(double*)(f)) = ( (*(double*)(args[0])) * 1.8 ) + 32;
}

// a user defined display function to display both C and F values
void display(void **args, void *_) {
  printf( "observable was updated to %fC/%fF\n",
          *(double*)(args[0]), *(double*)(args[1]) );
}

//                 level
// observable_temp   0   <-- update
//    /      \
//   temp_f   |      1
//     \     /
//   displayer       2   --> printf

int main(void) {
  // init temp behavior
  observable_temp_init();

  // create a new behviour that convers C to F
  observable_t temp_f = observe(just(observable_temp), c2f, double);

  // simulate changes to temp
  temp_update(16);
  temp_update(17);
  temp_update(18);
  
  // so far no output
  
  // some debugging feedback
  printf( "current temp=%f | temp_f=%f\n",
          *(double*)observable_value(observable_temp),
          *(double*)observable_value(temp_f));

  // let's add an observer that displays the updates

  // display updates from now on using our display observer function
  observable_t displayer = observe(both(observable_temp, temp_f), display);

  temp_update(19);
  temp_update(20);
  temp_update(21);
  
  dispose(displayer);

  temp_update(22);
  temp_update(23);
  temp_update(24);

  // some debugging feedback
  printf( "current temp=%f | temp_f=%f\n",
          *(double*)observable_value(observable_temp),
          *(double*)observable_value(temp_f));

  dispose(temp_f);

  exit(EXIT_SUCCESS);
}
