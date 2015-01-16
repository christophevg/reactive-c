// support for simulating the network and clock

#ifndef __SIMULATION_H
#define __SIMULATION_H

#include "network_observable.h"

void simulate_network_send(address_t, address_t, payload_t);
void simulate_clock_tick(int);

#endif
