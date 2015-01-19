// network observable

#ifndef __NETWORK_OBSERVABLE_H
#define __NETWORK_OBSERVABLE_H

#include "observer.h"

// the network has simplified addressing (int) and payloads (int)
typedef int address_t;
typedef int payload_t;

typedef struct msg{
  address_t from;
  address_t to;
  payload_t payload;
} *msg_t;

// we expose a global observable network
observable_t network_observable;

void network_init(void);
void network_send(msg_t);

#endif
