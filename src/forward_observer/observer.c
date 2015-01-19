#include "observer.h"
#include "stdlib.h"

// keeping internals of struct private

typedef struct observing {
  observer_t       function;
  void*            env;
  struct observing *next;
} *observing_t;

typedef struct observable {
  observing_t observers; // the root of the linked list of observers
  observing_t last;      // the last of the linked list of observers
} observable;

// implementation of exposed API

observable_t observable_new(void) {
  observable_t observable = malloc(sizeof(struct observable));
  observable->observers = NULL;
  observable->last      = NULL;
  return observable;
}

void observable_add(observable_t observable, observer_t observer, void *env) {
  if(observable->observers == NULL) {
    // empty list
    observable->observers = malloc(sizeof(struct observing));
    observable->last = observable->observers;
  } else {
    // next
    observable->last->next = malloc(sizeof(struct observing));
    observable->last = observable->last->next;
  }
  observable->last->function = observer;
  observable->last->env      = env;
  observable->last->next     = NULL;
}

void observable_remove(observable_t observable, observer_t observer, void *env) {
  // empty list
  if(observable->observers == NULL) { return; }
  // first observer
  if(observable->observers->function == observer &&
     observable->observers->env      == env )
  {
    observable->observers = observable->observers->next;
    if(observable->observers == NULL) {  // first and only
      observable->last = NULL;
    }
    return;
  }
  observing_t observing = observable->observers;
  while(observing->next) {
    if(observing->next->function == observer && observing->next->env == env) {
      observing->next = observing->next->next;
      if(observing->next == NULL) {
        observable->last = observing;
      }
      free(observer);
      return;
    }
    observing = observing->next;
  }
}

void observable_notify(observable_t observable, void *data) {
  observing_t observing = observable->observers;
  while(observing) {
    observing->function(observing->env, data);
    observing = observing->next;
  }
}
