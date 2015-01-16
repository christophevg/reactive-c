// support for simulating the network and clock

#include <stdio.h>

#include "observer.h"
#include "simulation.h"
#include "clock_observable.h"

void simulate_network_send(msg_t msg) {
  printf("nw    : %d -> %d : %d\n", msg.from, msg.to, msg.payload);
  network_send(msg);
}

// we provide a function to simulate clock activity
void simulate_clock_tick(int ms) {
  printf("clock : increasing time by %d to %d\n", ms, clock_now());
  clock_tick(ms);
}
