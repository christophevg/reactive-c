// forward observer module interface

#ifndef __FORWARD_OBSERVER_H
#define __FORWARD_OBSERVER_H

// the public forward observer type
typedef struct forward_observer *forward_observer_t;

// create a new observer for the given environment
forward_observer_t forward_observer_create(int, int);

// cleanup
void forward_observer_terminate(forward_observer_t);

#endif
