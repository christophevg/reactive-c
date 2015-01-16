// forward-observer pattern implemented in C

// read the README before reading this code ;-)

#include <stdio.h>
#include <stdlib.h>

#include "forward_observer.h"
#include "simulation.h"

int main(void) {
  // initialize the forward observer
  forward_observer_init();

  // start simulation

  // 1 sends 200 to 2, which is my parent. start observing!
  simulate_network_send((msg_t){1, 2, 100});
  simulate_clock_tick(100);
  // 2 forwards the message, ok -> stop observing!
  simulate_network_send((msg_t){2, 3, 100});
  simulate_clock_tick(100);
  // 3 sends 200 to 2, which is my parent. start observing!
  simulate_network_send((msg_t){3, 2, 200});
  simulate_clock_tick(500);  // no timeout
  simulate_clock_tick(500);  // timeout

  // terminate the forward observer
  forward_observer_terminate();

  exit(EXIT_SUCCESS);
}
