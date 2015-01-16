// support for simulating the network and clock

#ifndef __SIMULATION_H
#define __SIMULATION_H

#include "network_observable.h"

void simulate_network_send(msg_t);
void simulate_clock_tick(int);

#endif
