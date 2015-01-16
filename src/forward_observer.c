// forward observer module

#include <stdio.h>
#include <stdlib.h>

#include "forward_observer.h"

#include "network_observable.h"
#include "clock_observable.h"

// structure to hold environment/configuration information for an observer
typedef struct forward_env {
  int parent;
  int timeout;
} *forward_env_t;

// structure to hold all messages we're expecting a forward for.
typedef struct forward {
  payload_t payload;
  int       end;
} forward_t;

typedef struct forward_observer {
  forward_env_t env;
} forward_observer;

// temporary fixed size array for storing forwards we're tracking
#define MAX_FORWARD_COUNT 5
forward_t forwards[MAX_FORWARD_COUNT];

// first observer: messages sent to my parent
void _observe_msg_to_parent(void* _env, void* _msg) {
  forward_env_t env = (forward_env_t)(_env);
  msg_t         msg = (msg_t)(_msg);
  if(msg->to == env->parent) {
    printf("client: message to parent observed (payload=%d)\n\n", msg->payload);
    for(int i=0;i<MAX_FORWARD_COUNT;i++) {
      if(forwards[i].end == 0) {
        forwards[i] = (forward_t){ msg->payload, clock_now() + env->timeout };
        return;
      }
    }
    printf("FAILED to store forward info.\n");
  }
}

// second observer: messages sent by parent
void _observe_msg_from_parent(void* _env, void* _msg) {
  forward_env_t env = (forward_env_t)(_env);
  msg_t         msg = (msg_t)(_msg);
  if(msg->from == env->parent) {
    printf("client: message from parent observed (payload=%d).\n", msg->payload);
    for(int i=0; i<MAX_FORWARD_COUNT;i++) {
      if(msg->payload == forwards[i].payload) {
        printf("        yes, we were looking for it.\n");
        if(forwards[i].end < clock_now()) {
          printf("        it took too long. should have been cleared.\n\n");
        } else {
          printf("        ok, clearing forward.\n\n");
          forwards[i] = (forward_t){ 0, 0 };
        }
        return;
      }
    }
  }
}

// third observer: timeout detection
void _observe_clock_for_timeouts(void* _env, void* _null) {
  for(int i=0; i<MAX_FORWARD_COUNT;i++) {
    if(forwards[i].end > 0) {
      if(forwards[i].end <= clock_now()) {
        printf("client: TIMEOUT for payload %d\n", forwards[i].payload);
        forwards[i] = (forward_t){ 0, 0 };
      } else {
        printf("client: continue waiting for forward of %d for %d ms\n\n",
               forwards[i].payload, forwards[i].end - clock_now());
      }
    }
  }
}

int _initialized = 0;

void _forward_observers_init(void) {
  // init observers we're going to use
  clock_init();
  network_init();
  
  // init forwarders
  for(int i=0; i<MAX_FORWARD_COUNT;i++) {
    forwards[i] = (forward_t){ 0, 0 };
  }
  
  _initialized = 1;
}

forward_observer_t forward_observer_create(int parent, int timeout) {
  // lazy initialization
  if( ! _initialized ) { _forward_observers_init(); }
  forward_observer_t observer = malloc(sizeof(struct forward_observer));
  observer->env = malloc(sizeof(struct forward_env));
  observer->env->parent = parent;
  observer->env->timeout = timeout;

  // setup observers
  observable_add(network_observable, _observe_msg_to_parent,      (void*)observer->env);
  observable_add(network_observable, _observe_msg_from_parent,    (void*)observer->env);
  observable_add(clock_observable,   _observe_clock_for_timeouts, (void*)observer->env);
  
  return observer;
}

void forward_observer_terminate(forward_observer_t observer) {
  observable_remove(network_observable, _observe_msg_to_parent,      (void*)observer->env);
  observable_remove(network_observable, _observe_msg_from_parent,    (void*)observer->env);
  observable_remove(clock_observable,   _observe_clock_for_timeouts, (void*)observer->env);
  free(observer->env);
  free(observer);
}
