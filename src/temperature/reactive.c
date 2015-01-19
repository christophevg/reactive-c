// reactive c

#include <stdlib.h>

#include "reactive.h"

// linked list item (LI) for observers
typedef struct observer_li {
  observable_t observer;
  struct observer_li *next;
} *observer_li_t;

typedef struct observable {
  void *value;                   // cached                             <----+
  observer_t    generator;       // function that given in input produces __|
  observable_t  observing;       // the observable we're observing
  observer_li_t observers;       // root of linked list with observers
  observer_li_t last_observer;   // helper for faster addition
} observable;

// constructors

observable_t observable_from_value(void* value) {
  observable_t observable   = malloc(sizeof(struct observable));
  observable->value         = value;
  observable->generator     = NULL;
  observable->observing     = NULL;
  observable->observers     = NULL;
  observable->last_observer = NULL;
  return observable;
}

observable_t observable_from_observer(observer_t observer, int size) {
  observable_t observable   = malloc(sizeof(struct observable));
  observable->value         = (void*)malloc(size);
  observable->generator     = observer;
  observable->observing     = NULL;
  observable->observers     = NULL;
  observable->last_observer = NULL;
  return observable;
}

// extract current value from this observable

void *observable_value(observable_t observable) {
  return observable->value;
}

// make a observer observe an observable, turning it in an observable itself

observable_t observe(observable_t observable, observer_t observer, int size) {
  // step 2: turn the observer into an observable
  observable_t observable_observer = observable_from_observer(observer, size);
  // add a back link
  observable_observer->observing = observable;

  // step 1: add the observer to the observable
  if(observable->last_observer == NULL) {
    // first observer
    observable->observers = malloc(sizeof(struct observer_li));
    observable->last_observer = observable->observers;
  } else {
    // add one
    observable->last_observer->next = malloc(sizeof(struct observer_li));
    observable->last_observer = observable->last_observer->next;
  }
  // fill info
  observable->last_observer->observer = observable_observer;
  observable->last_observer->next     = NULL;

  return observable_observer;
}

void observe_update(observable_t observable) {
  // if we have a generator and something we're observing, get the value from
  // the observed and feed it through the generator into our own value
  if(observable->generator != NULL && observable->observing != NULL) {
    observable->generator(observable_value(observable->observing), observable->value);
  }
  // notify all our observers to do the same
  observer_li_t observer = observable->observers;
  while(observer) {
    observe_update(observer->observer);
    observer = observer->next;
  }
}
