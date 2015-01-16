// forward-observer pattern implemented in C

// read the README before reading this code ;-)

#include <stdlib.h>

#include "forward_observer.h"
#include "simulation.h"

int main(void) {
  // create a forward observer
  forward_observer_t forward_observer = forward_observer_create(2, 1000);

  // start simulation

  // 1 sends 200 to 2, which is my parent. start observing!
  simulate_network_send( 1, 2, 100 );
  simulate_clock_tick(100);
  // 2 forwards the message, ok -> stop observing!
  simulate_network_send( 2, 3, 100 );
  simulate_clock_tick(100);
  // 3 sends 200 to 2, which is my parent. start observing!
  simulate_network_send( 3, 2, 200 );
  simulate_clock_tick(500);  // no timeout
  simulate_clock_tick(500);  // timeout

  // terminate the forward observer
  forward_observer_terminate(forward_observer);

  exit(EXIT_SUCCESS);
}
