// network observable

#include "network_observable.h"

// create the global network_observable

void network_init(void) {
  network_observable = observable_new();
}

void network_send(msg_t msg) {
  observable_notify(network_observable, (void*)&msg);
}
