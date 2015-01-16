// support for simulating the network and clock

#include <stdio.h>
#include <stdlib.h>

#include "observer.h"
#include "simulation.h"
#include "clock_observable.h"

void simulate_network_send(address_t from, address_t to, payload_t payload) {
  printf("nw    : %d -> %d : %d\n", from, to, payload);
  msg_t msg = malloc(sizeof(struct msg));
  msg->from    = from;
  msg->to      = to;
  msg->payload = payload;
  network_send(msg);
}

// we provide a function to simulate clock activity
void simulate_clock_tick(int ms) {
  printf("clock : increasing time by %d to %d\n", ms, clock_now());
  clock_tick(ms);
}
