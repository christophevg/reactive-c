// reactive c

#include <stdio.h>

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "reactive.h"

// linked list item (LI) for observables. "next" can't be added to observable
// because each observable can be grouped with other observers or observed
// observables.
typedef struct observable_li {
  observable_t         ob;
  struct observable_li *next;
} *observable_li_t;

// the list is also packed in a struct, containing the root/first list item and
// a reference to the last, for faster additions.
struct observables {
  observable_li_t first;
  observable_li_t last;
} observables;

// constructor and accessors for an observables list
observables_t _new_observables_list(void) {
  observables_t list = malloc(sizeof(struct observables));
  list->first = NULL;
  list->last = NULL;
  return list;
}

void _add_observable(observables_t list, observable_t observable) {
  if(list->first == NULL) {
    assert(list->last == NULL);
    // prepare first list item
    list->first = malloc(sizeof(struct observable_li));
    list->last = list->first;
  } else {
    // prepare next list item
    list->last->next = malloc(sizeof(struct observable_li));
    list->last = list->last->next;
  }
  list->last->ob   = observable;
  list->last->next = NULL;
}

void _remove_observable(observables_t list, observable_t observable) {
  if(list->first == NULL) {
    assert(list->last == NULL);
    return; // empty list
  }
  observable_li_t head = list->first;
  if(head->ob == observable) {
    // first in the list, remove the head
    list->first = head->next;
    free(head);
    if(list->first == NULL) { list->last = NULL; } // it was also the only item
    return;
  }
  // look further down the list
  while(head->next && head->next->ob != observable) { head = head->next; }
  if(head->next) {
    observable_li_t hit = head->next;
    head->next = hit->next;
    free(hit);
    if(head->next == NULL) { list->last = head; } // it was also the last item
  }
}

void _clear_observables(observables_t list) {
  observable_li_t item = list->first;
  while(list->first) {
    item = list->first;
    list->first = list->first->next;
    free(item);
  }
  list->first = NULL;
  list->last  = NULL;
}

// observables can have several properties, which influences the internal
// workings.
enum properties {
  UNKNOWN     = 0,
  VALUE       = 1,
  OBSERVER    = 3,
  OUT_IS_SELF = 4, // don't provide value but the entire observable as out param
  DISPOSED    = 8  // used to mark an observable as ready to be removed
};

typedef struct observable {
  int            prop;           // internal properties
  void           *value;         // cached or pointer to observed value <---+
  observer_t     process;        // function that given input produces _____|
  observables_t  observeds;      // first of observed observables
  void           **args;         // array of pointers to values of observeds
  int            level;          // the level in the dependecy graph
  observables_t  observers;      // first of observers
  observable_t   parent;         // helper field for storing creating parent
  // scripting support (old fragments)
  observable_t   next;           // if this observable is done, start the next
  // events
  observable_callback_t  on_dispose;
  observable_callback_t  on_activation;
} observable;

// private functionality

// constructor for empty observable
observable_t _new() {
  observable_t this   = malloc(sizeof(struct observable));
  this->prop          = UNKNOWN;
  this->value         = NULL;
  this->process       = NULL;
  this->observeds     = _new_observables_list();
  this->args          = NULL;
  this->level         = 0;
  this->observers     = _new_observables_list();
  this->parent        = NULL;
  this->next          = NULL;
  this->on_dispose    = NULL;
  this->on_activation = NULL;
  return this;
}

// recompute the level for this observable. do this by computing the maximum
// level for all observed observables + 1
observable_t _update_level(observable_t this) {
  observable_li_t observed = this->observeds->first;
  int level = 0;
  while(observed) {
    if(observed->ob->level > level) { level = observed->ob->level; }
    observed = observed->next;
  }
  this->level = level + 1;
  return this;
}

// recompute the arguments pointer list for this observable. do this by storing
// all value pointers of our own observed observables in an array.
observable_t _update_args(observable_t this) {
  // compute number of observed observables
  observable_li_t observed = this->observeds->first;
  int count = 0;
  while(observed) {
    count++;
    observed = observed->next;
  }
  // optionally free existing list
  if(this->args != NULL) { free(this->args); }
  
  // allocate memory to hold cached list of pointers to observed values
  this->args = malloc(sizeof(void*)*count);
  
  // copy pointers to values
  observed = this->observeds->first;
  count = 0;
  while(observed) {
    this->args[count] = observed->ob->value;
    count++;
    observed = observed->next;
  }

  return this;
}

// remove all links to observers
observable_t _clear_observers(observable_t this) {
  // remove back-links from our observers
  for(observable_li_t item = this->observers->first; item; item = item->next) {
    _remove_observable(item->ob->observeds, this);
  }
  _clear_observables(this->observers);
  return this;
}

// remove all links to observed observables
observable_t _clear_observeds(observable_t this) {
  // remove back-links from our observeds
  for(observable_li_t item = this->observeds->first; item; item = item->next) {
    _remove_observable(item->ob->observers, this);
  }
  _clear_observables(this->observeds);
  return this;
}

// actually free the entire observable structure
void _free(observable_t this) {
  _clear_observers(this);
  _clear_observeds(this);
  if(this->args)  { free(this->args);  }
  if(this->value) { free(this->value); }
  free(this);
}

// check all observers and remove those that are marked for disposal
observable_t _gc(observable_t this) {
  if(this->observers->first == NULL) {
    assert(this->observers->last == NULL);
    return this;
  }

  // garbage collect disposed observers
  observable_li_t item = this->observers->first;
  if(item->ob->prop & DISPOSED) { // first got disposed
    _free(item->ob);  // free entire observable (including all links)
    return this;
  }
  // not first, look further
  while(item->next) {
    while(item->next && (item->next->ob->prop & DISPOSED)) {
      observable_li_t temp = item->next;
      item->next = item->next->next;
      _free(item->ob);
      free(item);
    }
    item = item->next;
  }
  return this;
}

// internal observer function to merge value updates to multiple observables
// into one "merged" observable observer.
void _merge(void **args, void* this) {
  observable_t merged = ((observable_t)this)->parent;
  merged->value = args[0];

  // cached args are out of date, because we're modifying the pointer itself
  // force refresh cache on all observers of merged_ob
  observable_li_t iter = merged->observers->first;
  while(iter) {
    _update_args(iter->ob);
    iter = iter->next;
  }

  observe_update(merged);
}

observable_t _step(observable_t script) {
  observable_t step = script->next;
  if(step == NULL) { return script; } // end of script

  // prepare for next fragment (before _activate because it frees the fragment)
  script->next = step->next;

  // start
  start(step);
  if(step->parent && step->parent->on_activation) {
    step->parent->on_activation(step);
  }

  // observe the new step (future use)
  // _add_observable(script->observeds, step);
  
  return script;
}

void _finalize_await(void **args, void *this) {
  // dispose ourselves
  dispose((observable_t)this);

  // tell the script to move to the next step
  _step(((observable_t)this)->parent);
}

// turns a variadic list of observables into an linked list. this is a helper
// function to allow calling observe(all(...) and pass a variable list of
// observables)
observables_t __all(int count, ...) {
  va_list       ap;
  observables_t list = _new_observables_list();
  
  // import observables
  va_start(ap, count);
  for(int i=0; i<count; i++) {
    _add_observable(list, va_arg(ap, observable_t));
  }
  va_end(ap);
  
  // return casted version
  return list;
}

// an ExternalValueObservable simply stores a pointer to some value in memory,
// which is managed externally. when this value is updated, the observer_update
// function should be called to activate the reactive behaviour associated with
// it through this observable.
observable_t __observing_value(void* value) {
  observable_t this = _new();
  this->prop        = VALUE;
  this->value       = value;
  return this;
}

// create an observable observer (function), storing the resulting value in a
// memory location of size.
observable_t __observing(observables_t observeds, observer_t observer, int size) {
  // turn the observer into an observable
  observable_t this = _new();
  this->prop        = OBSERVER;
  this->value       = (void*)malloc(size);
  this->process     = observer;
  this->observeds   = observeds;    // this is already partial in the graph
                                    // but cannot be used, no back-links
  return this;
}

// public interface

// actually connect the observable in the dependecy graph
observable_t start(observable_t this) {
  // step 1: add observeds and update the arguments list and our level in the
  // dependency graph
  _update_args(this);
  _update_level(this);

  // step 2: add a back-link to the observer to all observeds
  observable_li_t iter = this->observeds->first;
  while(iter) {
    _add_observable(iter->ob->observers, this);
    iter = iter->next;
  }
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
  if(this->on_dispose) { this->on_dispose(this); }
  this->prop |= DISPOSED;
}

// trigger for (external) update of observable
void observe_update(observable_t this) {
  // if we're marked for disposal (externally), we don't perform any processing
  // one of our observed parents will garbage collect us (sometime)
  if(this->prop & DISPOSED) { return; }

  // if we have a process execute it
  if(this->process != NULL) {
    // if the process is the internal _merge, we pass the parent, not the value
    if(this->prop & OUT_IS_SELF) {
      this->process(this->args, this);
    } else {
      this->process(this->args, this->value);
    }
  }

  // the logic in the process might have marked us for disposal, still we allow
  // the event flow to continue down to our observers one more time.

  // notify all our observers to do the same, but only those that are directly
  // dependant on us, so with level = our level + 1. those with even higher
  // levels are dependant on other levels below us and will be triggered as
  // soon as their parents all have been updated
  observable_li_t observer = this->observers->first;
  if(observer == NULL) {
    assert(this->observers->last == NULL);
  }
  while(observer) {
    if(observer->ob->level == this->level + 1) {
      observe_update(observer->ob);
    }
    observer = observer->next;
  }

  // perform garbage collection (on our observers)
  _gc(this);
}

// extract current value from this observable
void *observable_value(observable_t observable) {
  return observable->value;
}

// merging support

// create a single observable observer from a list of observed observables.
observable_t __merge(observables_t observeds) {
  observable_t merged = __observing_value(NULL);
  observable_li_t observed = observeds->first;
  while(observed) {
    observable_t tmp = observe(just(observed->ob), _merge);
    tmp->prop |= OUT_IS_SELF;
    tmp->parent = merged;
    observed = observed->next;
  }
  return merged;
}

// scripting support

// constructor for observer that wait until another observer emits
observable_t await(observable_t observer) {
  observable_t this = __observing(just(observer), _finalize_await, 0);
  this->prop        = OUT_IS_SELF;
  return this;
}

// constructor for a script, consisting of count inactive observables
observable_t __script(int count, ...) {
  if(count<1) { return NULL; }
  
  observable_t script = _new();
   
  va_list ap;
  va_start(ap, count);

  // first due to count >= 1
  script->next = va_arg(ap, observable_t);
  observable_t step = script->next;
  step->parent = script;

  // remaining fragments
  for(int i=1; i<count; i++) {
    step->next = va_arg(ap, observable_t);
    step = step->next;
    step->parent = script;
  }
  va_end(ap);

  return script;
}

observable_t run(observable_t script) {
  return _step(script);  // activates the first fragment in a consistent way
}
