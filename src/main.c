// forward-observer pattern implemented in C

// read the README before reading this code ;-)

#include <stdio.h>
#include <stdlib.h>

// some configuration

// my parent is the node I'm sending my messages to, to have them propagated
// through the meshed network

#define my_parent 2

// if my parent doesn't forward a message within the set timeout value, it's
// considered non-cooperative.

#define timeout 1000

// the network has simplified addressing (int) and payloads (int)
typedef int address_t;
typedef int payload_t;

typedef struct {
  address_t from;
  address_t to;
  payload_t payload;
} msg_t;

// observers are in fact function pointers to callbacks that handle events.

typedef void (*nw_observer_t)(msg_t msg);

// observables are three functions and one data structure. one function allows
// adding an observer to the data structure. the second function allows removal
// of an observer. the third function is triggering an event to be dispatched to
// the registered observer.

// first observable: the network
#define MAX_NW_OBSERVERS 3
nw_observer_t network_observers[MAX_NW_OBSERVERS];    // simple hard-coded list

void nw_observer_init(void) {
  // clear all pointers
  for(int i=0;i<MAX_NW_OBSERVERS;i++) {
    network_observers[i] = NULL;
  }
}

void nw_add_observer(nw_observer_t observer) {
  // find a NULL and allocate it
  for(int i=0;i<MAX_NW_OBSERVERS;i++) {
    if(network_observers[i] == NULL) {
      network_observers[i] = observer;
      return;
    }
  }
  printf("FAILED to add observer to network.\n");
}

void nw_remove_observer(nw_observer_t observer) {
  // find a NULL and allocate it
  for(int i=0;i<MAX_NW_OBSERVERS;i++) {
    if(network_observers[i] == observer) {
      network_observers[i] = NULL;
      return;
    }
  }
  printf("FAILED to remove observer to network.");
}

void nw_handle_msg(msg_t msg) {
  for(int i=0;i<MAX_NW_OBSERVERS;i++) {
    if(network_observers[i] != NULL) {
      network_observers[i](msg);
    }
  }
}

// we provide a function to add network activity
void nw_send(msg_t msg) {
  printf("nw    : %d -> %d : %d\n", msg.from, msg.to, msg.payload);
  // this gets injected into the handling of messages
  nw_handle_msg(msg);
}

// second observable: clock
// we use a virtual clock to perform the simulation

typedef void (*clock_observer_t)(void);

int clock = 0;

#define MAX_CLOCK_OBSERVERS 3
clock_observer_t clock_observers[MAX_CLOCK_OBSERVERS];

void clock_observer_init(void) {
  // clear all pointers
  for(int i=0;i<MAX_CLOCK_OBSERVERS;i++) {
    clock_observers[i] = NULL;
  }
}

void clock_add_observer(clock_observer_t observer) {
  // find a NULL and allocate it
  for(int i=0;i<MAX_CLOCK_OBSERVERS;i++) {
    if(clock_observers[i] == NULL) {
      clock_observers[i] = observer;
      return;
    }
  }
  printf("FAILED to add clock observer.\n");
}

void clock_remove_observer(clock_observer_t observer) {
  // find a NULL and allocate it
  for(int i=0;i<MAX_CLOCK_OBSERVERS;i++) {
    if(clock_observers[i] == observer) {
      clock_observers[i] = NULL;
      return;
    }
  }
  printf("FAILED to remove clock observer.");
}

void clock_handle_tick(void) {
  for(int i=0;i<MAX_CLOCK_OBSERVERS;i++) {
    if(clock_observers[i] != NULL) {
      clock_observers[i]();
    }
  }
}

// we provide a function to simulate clock activity
void clock_tick(int ms) {
  clock += ms;
  printf("clock : increasing time by %d to %d\n", ms, clock);
  clock_handle_tick();
}

// CLIENT-IMPLEMENTATION STARTS HERE ....

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
void observe_msg_to_parent(msg_t msg) {
  if(msg.to == my_parent) {
    printf("client: message to parent observed (payload=%d)\n\n", msg.payload);
    for(int i=0;i<MAX_FORWARD_COUNT;i++) {
      if(forwards[i].end == 0) {
        forwards[i] = (forward_t){ msg, clock + timeout };
        return;
      }
    }
    printf("FAILED to store forward info.\n");
  }
}

// second observer: messages sent by parent
void observe_msg_from_parent(msg_t msg) {
  if(msg.from == my_parent) {
    printf("client: message from parent observed (payload=%d).\n", msg.payload);
    for(int i=0; i<MAX_FORWARD_COUNT;i++) {
      if(msg.payload == forwards[i].msg.payload) {
        printf("        yes, we were looking for it.\n");
        if(forwards[i].end < clock) {
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
void observe_clock_for_timeouts() {
  for(int i=0; i<MAX_FORWARD_COUNT;i++) {
    if(forwards[i].end > 0) {
      if(forwards[i].end <= clock) {
        printf("client: TIMEOUT for payload %d\n", forwards[i].msg.payload);
        forwards[i] = (forward_t){ (msg_t){ 0, 0, 0 }, 0 };
      } else {
        printf("client: continue waiting for forward of %d for %d ms\n\n",
               forwards[i].msg.payload, forwards[i].end - clock);
      }
    }
  }
}

int main(void) {
  nw_observer_init();
  forward_init();

  // setup observers
  nw_add_observer(observe_msg_to_parent);
  nw_add_observer(observe_msg_from_parent);
  clock_add_observer(observe_clock_for_timeouts);

  // simulate events
  nw_send((msg_t){1, 2, 100});
  // 1 sends 200 to 2, which is my parent. start observing!
  clock_tick(100);
  // 2 forwards the message, ok -> stop observing!
  nw_send((msg_t){2, 3, 100});
  clock_tick(100);
  // 3 sends 200 to 2, which is my parent. start observing!
  nw_send((msg_t){3, 2, 200}); 
  clock_tick(500);  // no timeout
  clock_tick(500);  // timeout

  exit(EXIT_SUCCESS);
}
