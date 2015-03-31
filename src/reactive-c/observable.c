#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "iterator.h"

#include "debug.h"

// constructor for empty observable
observable_t _new(char *label) {
  observable_t this   = malloc(sizeof(struct observable));
  this->label         = label;
  this->prop          = UNKNOWN;
  this->value         = NULL;
  this->type_size     = 0;
  this->process       = NULL;
  this->observeds     = NULL; // provided by external allocation (__each)
  this->args          = NULL;
  this->level         = 0;
  this->observers     = observables_new();
  this->validate      = NULL;
  this->parent        = NULL;
  this->next          = NULL;
  this->on_dispose    = NULL;
  this->on_activation = NULL;
  return this;
}

// actually free the entire observable structure
void _free(observable_t this) {
  if(this == NULL) { return; }
  _debug("FREEING", this);
  _clear_observers(this); free(this->observers);
  if(this->observeds) { _clear_observeds(this); free(this->observeds); }
  if(this->args)  { free(this->args);  this->args  = NULL; }
  if(!_is_value(this) && this->value) { free(this->value); this->value = NULL; }
  free(this);
}

// recompute the level for this observable. do this by computing the maximum
// level for all observed observables + 1
observable_t _update_level(observable_t this) {
  int level = 0;
  foreach(observable_li_t, iter, this->observeds) {
    if(iter->ob->level > level) { level = iter->ob->level; }
  }
  this->level = level + 1;
  return this;
}

// update observers' arguments - probably because this' value changed location
void _update_observers_args(observable_t this) {
  foreach(observable_li_t, iter, this->observers) {
    _update_args(iter->ob);
  }
}

// recompute the arguments pointer list for this observable. do this by storing
// all value pointers of our own observed observables in an array.
observable_t _update_args(observable_t this) {
  // optionally free existing list
  if(this->args) { free(this->args); }
  
  // allocate memory to hold cached list of pointers to observed values
  this->args = malloc(sizeof(unknown_t) * observables_count(this->observeds));
  
  // copy pointers to values
  int i = 0;
  foreach(observable_li_t, iter, this->observeds) {
    this->args[i++] = iter->ob->value;
  }

  return this;
}

// remove all links to observers
observable_t _clear_observers(observable_t this) {
  // remove back-links from our observers
  foreach(observable_li_t, iter, this->observers) {
    observables_remove(iter->ob->observeds, this);
  }
  observables_clear(this->observers);
  _debug("CLEARED OBSERVERS", this);
  return this;
}

// remove all links to observed observables
observable_t _clear_observeds(observable_t this) {
  // remove back-links from our observeds
  foreach(observable_li_t, iter, this->observeds) {
    observables_remove(iter->ob->observers, this);
  }
  observables_clear(this->observeds);
  _debug("CLEARED OBSERVEDS", this);
  return this;
}

// the trash is another list of observables
struct observables _bin = { NULL, NULL };
observables_t bin = &_bin;

// add an observable to the trash
#define _trash(o) observables_add(bin, o)

// actually free all observables in the bin
void empty_trash() {
  foreach(observable_li_t, item, bin) {
    observable_t observable = item->ob;
    observables_remove(bin, observable);
    _free(observable);
  }
}

///////////

// an ExternalValueObservable simply stores a pointer to some value in memory,
// which is managed externally. when this value is updated, the observer_update
// function should be called to activate the reactive behaviour associated with
// it through this observable.
observable_t __observing_value(char *label, unknown_t value, int size) {
  observable_t this = _new(label);
  this->prop        = VALUE;
  this->value       = value;
  this->type_size   = size;
  return this;
}

// create an observable observer (function), storing the resulting value in a
// memory location of size.
observable_t __observing(char *label, observables_t observeds,
                         observer_t observer, int size)
{
  // create an observing observer with the fiven observing handler
  observable_t this = _new(label);
  this->prop        = OBSERVER;
  this->value       = size ? (unknown_t)malloc(size) : NULL;
  this->type_size   = size;
  this->process     = observer;

  // set its observed observables
  _set_observeds(this, observeds);

  return this;
}

void _set_observeds(observable_t this, observables_t observeds) {
  // step 1: set the obseveds
  this->observeds = observeds;    // this is already partial in the graph
                                  // but cannot be used, no back-links

  // step 2: update the arguments list and our level in the dependency graph
  _update_args(this);
  _update_level(this);

  // step 3: add a back-link to the observer to all observeds
  foreach(observable_li_t, iter, this->observeds) {
    observables_add(iter->ob->observers, this);
  }
}

// public interface

observable_t suspend(observable_t this) {
  if(!_is_suspended(this)) {
    this->prop |= SUSPENDED;
    _dont_propagate(this);
    _debug("SUSPEND", this);
  }
  return this;
}

observable_t unsuspend(observable_t this) {
  if(_is_suspended(this)) {
    this->prop &= ~SUSPENDED;
    _debug("UNSUSPEND", this);
  }
  return this;
}

observable_t delay(observable_t this) {
  this->prop |= DELAYED;
  _dont_propagate(this);
  _debug("DELAY", this);
  return this;
}

observable_t undelay(observable_t this) {
  this->prop &= ~DELAYED;
  _debug("UNDELAY", this);
  return this;
}

// actually activate the observable in the dependecy graph
observable_t start(observable_t this) {
  _debug("STARTING", this);
  // step 1: cancel suspension
  unsuspend(this);
  // step 2: if we have delayed observeds, un-delay them
  foreach(observable_li_t, iter, this->observeds) {
    undelay(iter->ob);
  }
  _debug("STARTED", this);
  return this;
}

// add a callback to the observable, triggered when it is disposed
observable_t on_dispose(observable_t this, observable_callback_t callback) {
  this->on_dispose = callback;
  return this;
}

// add a callback to the observable, triggered when it is activated
observable_t on_activation(observable_t this, observable_callback_t callback) {
  this->on_activation = callback;
  return this;
}

// marks an observable for disposing, which is honored when an update-push is
// executed on it.
void dispose(observable_t this) {
  if(this == NULL) { return; }
  this->prop |= DISPOSED;
  _debug("DISPOSE", this);
  if(this->on_dispose) { 
    this->on_dispose(this);
  }
  _trash(this);
}

// trigger for (external) update of observable
void _observe_update(observable_t this, observable_t source) {
  // if we're marked for disposal (externally), we don't perform any processing
  // one of our observed parents will garbage collect us (sometime)
  // TODO: only true for observables that observe and can be updated
  if(_is_disposed(this)) { return; }
  
  // if we're suspended, we ignore this update
  if(_is_suspended(this)) { return; }
  
  // if we have a filter, check if we want to propagate
  if(this->validate && source){
    if(! this->validate(source->value) ) {
      _dont_propagate(this);
      return;
    } else {
      _propagate(this);
    }
  }

  // if we have a process execute it (IF WE'RE NOT OUR OWN SOURCE)
  if(this->process && this != source) {
    // do we pass the value or the object itself?
    struct observation ob = {
      .self       = this,
      .source     = source,
      .observeds  = this->args,
      ob.observer = this->value
    };
    this->process(&ob);
  }

  // the logic in the process might have marked us for disposal, still we allow
  // the event flow to continue down to our observers one more time.

  // unless we don't want to propagate an update...
  if(_is_propagating(this)) {
    // notify all our observers to do the same, but only those that are directly
    // dependant on us, so with level = our level + 1. those with even higher
    // levels are dependant on other levels below us and will be triggered as
    // soon as their parents all have been updated
    foreach(observable_li_t, iter, this->observers) {
      if( ! _is_disposed(iter) ) {
        if(iter->ob->level == this->level + 1) {
          _observe_update(iter->ob, this);
        }
      }
    }
  }
}

struct observable_handler_li {
  observable_handler_t         handler;
  struct observable_handler_li *next;
};

struct observable_handler_li *update_handlers = NULL;

void _on_update(observable_handler_t handler) {
  struct observable_handler_li *item =
    malloc(sizeof(struct observable_handler_li));
  item->handler   = handler;
  item->next      = update_handlers;
  update_handlers = item;
}

// public method can only trigger update, not be a source
void observe_update(observable_t observable) {
  _debug("UPDATE", observable);
  _observe_update(observable, NULL);
  // call all registered handlers
  for(struct observable_handler_li *item = update_handlers; item; item = item->next) {
    item->handler(observable);
  }
  // empty the trash
  empty_trash();
}

void stop_observing(void) {
  empty_trash();
  free(update_handlers);
}

// extract current value from this observable
unknown_t observable_value(observable_t observable) {
  return observable->value;
}

// value manipulation support

void observable_value_copy(observable_t src, observable_t trg) {
  // we don't know what we're copying, but we know the size ;-)
  // only copy when internal memory space is the same
  assert(src->type_size == trg->type_size);
  memcpy(trg->value, src->value, src->type_size);
}

void __set(observable_t this, void* value, int size) {
  assert(this->type_size == size);
  memcpy(this->value, value, size);
  observe_update(this);
}
