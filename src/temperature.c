// implementation of the temperature conversion behavior from the reactive
// programming survey ...

// we have a behavior temp that continuously presents the current temperature
// in degrees Celcius

#include <stdlib.h>

#include "unit/test.h"

#include "reactive-c/reactive.h"

// to observe a value/variable...
double temp = 123;

// ...we create an observable...
observable_t observable_temp;

// ... and link it to the value/variable.
void observable_temp_init() {
  observable_temp = observe(double, temp);
}

// when the value/variable is updated, we trigger the RP functionality
void temp_update(double update) {
  temp = update;
  // trigger update propagation
  observe_update(observable_temp);
}

// a user defined convertion between Celcius and Farenheit
void c2f(observation_t ob) {
  (*(double*)(ob->observer)) = ( (*(double*)(ob->observeds[0])) * 1.8 ) + 32;
}

// a user defined display function to display both C and F values
void display(observation_t ob) {
  capture_printf( "observable was updated to %fC/%fF\n",
                  *(double*)(ob->observeds[0]), *(double*)(ob->observeds[1]) );
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
  
  assert_no_output();
  
  assert_equal(
    *(double*)observable_value(observable_temp),
    18,
    "Expected temperature to be 18C. Observed %f.\n",
    *(double*)observable_value(observable_temp)
  );
  assert_equal(
    *(double*)observable_value(temp_f),
    18 * 1.8 + 32,
    "Expected temperature to be %f. Observed %f.\n",
    18 * 1.8 + 32,
    *(double*)observable_value(temp_f)
  );

  // let's add an observer that displays the updates from now on using our
  // display observer function
  observable_t displayer = observe(both(observable_temp, temp_f), display, void);

  temp_update(19);
  temp_update(20);
  temp_update(21);
  
  assert_output_was(
    "observable was updated to 19.000000C/66.200000F\n"
    "observable was updated to 20.000000C/68.000000F\n"
    "observable was updated to 21.000000C/69.800000F\n"
  );

  clear_output();

  dispose(displayer);

  temp_update(22);
  temp_update(23);
  temp_update(24);

  assert_no_output();
  
  assert_equal(
    *(double*)observable_value(observable_temp),
    24,
    "Expected temperature to be 18C. Observed %f.\n",
    *(double*)observable_value(observable_temp)
  );
  assert_equal(
    *(double*)observable_value(temp_f),
    24 * 1.8 + 32,
    "Expected temperature to be %f. Observed %f.\n",
    24 * 1.8 + 32,
    *(double*)observable_value(temp_f)
  );
  dispose(temp_f);

  exit(EXIT_SUCCESS);
}
