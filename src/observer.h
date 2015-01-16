// generic oo-style implementation of an observable

#ifndef __OBSERVER_H
#define __OBSERVER_H

// the generic observer type

// observers are function pointers to callbacks that handle events. original
// environment data and data wrt the event is passed through void pointers.

typedef void (*observer_t)(void*, void*);

// observables are a pointer to a struct

typedef struct observable *observable_t;

// constructor for an observable
observable_t observable_new   (void);

// add/remove observers to the observable
void         observable_add   (observable_t, observer_t, void*);
void         observable_remove(observable_t, observer_t, void*);

// let the observable notify all of its observers with the provided data
void         observable_notify(observable_t, void*);

#endif
