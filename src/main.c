// forward-observer pattern implemented in C

// read the README before reading this code ;-)

#include <stdio.h>
#include <stdlib.h>

#include "clock_observable.h"
#include "network_observable.h"

// some configuration

// my parent is the node I'm sending my messages to, to have them propagated
// through the meshed network

#define my_parent 2

// if my parent doesn't forward a message within the set timeout value, it's
// considered non-cooperative.

#define timeout 1000

// we're using a data structure to hold all messages we're expecting a forward
// for.

typedef struct forward {
  msg_t msg;
  int   end;
} forward_t;

#define MAX_FORWARD_COUNT 5
forward_t forwards[MAX_FORWARD_COUNT];

void forward_init(void) {
  for(int i=0; i<MAX_FORWARD_COUNT;i++) {
    forwards[i] = (forward_t){ (msg_t){ 0, 0, 0 }, 0 };
  }
}

// first observer: messages sent to my parent
void observe_msg_to_parent(void* msg_ptr) {
  msg_t msg = *(msg_t*)(msg_ptr);
  if(msg.to == my_parent) {
    printf("client: message to parent observed (payload=%d)\n\n", msg.payload);
    for(int i=0;i<MAX_FORWARD_COUNT;i++) {
      if(forwards[i].end == 0) {
        forwards[i] = (forward_t){ msg, clock_now() + timeout };
        return;
      }
    }
    printf("FAILED to store forward info.\n");
  }
}

// second observer: messages sent by parent
void observe_msg_from_parent(void* msg_ptr) {
  msg_t msg = *(msg_t*)(msg_ptr);
  if(msg.from == my_parent) {
    printf("client: message from parent observed (payload=%d).\n", msg.payload);
    for(int i=0; i<MAX_FORWARD_COUNT;i++) {
      if(msg.payload == forwards[i].msg.payload) {
        printf("        yes, we were looking for it.\n");
        if(forwards[i].end < clock_now()) {
          printf("        it took too long. should have been cleared.\n\n");
        } else {
          printf("        ok, clearing forward.\n\n");
          forwards[i] = (forward_t){ (msg_t){ 0, 0, 0 }, 0 };
        }
        return;
      }
    }
  }
}

// third observer: timeout detection
void observe_clock_for_timeouts(void* null_ptr) {
  for(int i=0; i<MAX_FORWARD_COUNT;i++) {
    if(forwards[i].end > 0) {
      if(forwards[i].end <= clock_now()) {
        printf("client: TIMEOUT for payload %d\n", forwards[i].msg.payload);
        forwards[i] = (forward_t){ (msg_t){ 0, 0, 0 }, 0 };
      } else {
        printf("client: continue waiting for forward of %d for %d ms\n\n",
               forwards[i].msg.payload, forwards[i].end - clock_now());
      }
    }
  }
}

int main(void) {
  clock_init();
  network_init();
  
  forward_init();

  // setup observers
  observable_add(network_observable, observe_msg_to_parent);
  observable_add(network_observable, observe_msg_from_parent);
  observable_add(clock_observable,   observe_clock_for_timeouts);

  // simulate events
  network_send((msg_t){1, 2, 100});
  // 1 sends 200 to 2, which is my parent. start observing!
  clock_tick(100);
  // 2 forwards the message, ok -> stop observing!
  network_send((msg_t){2, 3, 100});
  clock_tick(100);
  // 3 sends 200 to 2, which is my parent. start observing!
  network_send((msg_t){3, 2, 200});
  clock_tick(500);  // no timeout
  clock_tick(500);  // timeout

  exit(EXIT_SUCCESS);
}
