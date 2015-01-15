#include "observer.h"
#include "stdlib.h"

// keeping internals of struct private

typedef struct observer_item {
  observer_t observer;
  struct observer_item *next;
} *observer_item_t;

typedef struct observable {
  observer_item_t observers; // the root of the linked list of observers
  observer_item_t last;      // the last of the linked list of observers
} observable;

// implementation of exposed API

observable_t observable_new(void) {
  observable_t observable = malloc(sizeof(struct observable));
  observable->observers = NULL;
  observable->last      = NULL;
  return observable;
}

void observable_add(observable_t observable, observer_t observer) {
  if(observable->observers == NULL) {
    // empty list
    observable->observers = malloc(sizeof(struct observer_item));
    observable->last = observable->observers;
  } else {
    // next
    observable->last->next = malloc(sizeof(struct observer_item));
    observable->last = observable->last->next;
  }
  observable->last->observer = observer;
  observable->last->next     = NULL;
}

void observable_remove(observable_t observable, observer_t observer) {
  // empty list
  if(observable->observers == NULL) { return; }
  // first entree
  if(observable->observers->observer == observer) {
    observable->observers = observable->observers->next;
    if(observable->observers == NULL) {  // first and only
      observable->last = NULL;
    }
    return;
  }
  observer_item_t entree = observable->observers;
  while(entree->next) {
    if(entree->next->observer == observer) {
      entree->next = entree->next->next;
      if(entree->next == NULL) {
        observable->last = entree;
      }
      free(observer);
      return;
    }
    entree = entree->next;
  }
}

void observable_notify(observable_t observable, void *data) {
  observer_item_t entree = observable->observers;
  while(entree) {
    entree->observer(data);
    entree = entree->next;
  }
}
