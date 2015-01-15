// generic oo-style implementation of an observable

#ifndef __OBSERVER_H
#define __OBSERVER_H

// the generic observer type

// observers are function pointers to callbacks that handle events. data wrt the
// even is passed through a void pointer.

typedef void (*observer_t)(void *data);

// observables are a pointer to a struct

typedef struct observable *observable_t;

// constructor for an observable

observable_t observable_new   (void);
void         observable_add   (observable_t, observer_t);
void         observable_remove(observable_t, observer_t);
void         observable_notify(observable_t, void*);

#endif
