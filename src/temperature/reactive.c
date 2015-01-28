// reactive c

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "reactive.h"

// linked list item (LI) for observables. "next" can't be added to observable
// because each observable can be grouped with other observers or observed
// observables.
struct observable_li {
  observable_t    ob;
  observable_li_t next;
} observable_li;

// observables can have several properties, which influences the internal
// workings.
enum properties {
  UNKNOWN     = 0,
  VALUE       = 1,
  OBSERVER    = 3,
  OUT_IS_SELF = 4, // don't provide value but the entire observable as out param
  DISPOSED    = 8  // used to mark an observable as ready to be removed
};

// an observable has properties that are used internally to manage its
// functionality and/or lifetime. an observable contains a pointer to a value
// (for ExternalValueObservers). alternatively it contains a pointer to an
// adapter observer (function), which takes the current values of its observed
// observables and computes it own new value. these observed observables are
// stored in a linked list. because the current values of these observables as
// pointers, we can point to them too from an array. this array is a cached copy
// of the arguments passed to the adapter function, which now only needs to be
// computed once (when adding observed observables, through the observer()
// function). given the observed observables, we can compute the level within
// the dependency graph, being the max of the levels of the observed observable
// + 1. changes to this value need to trigger an update with all observers of an
// observable. a list of observing observables tracking this observable are also
// stored in a linked list. because observables can be created by means of a
// script, a link to the next fragement in the script is also stored in an
// observable.
typedef struct observable {
  int              prop;           // internal properties
  void             *value;         // cached or pointer to observed value <---+
  observer_t       adapter;        // function that given input produces _____|
  observable_li_t  observeds;      // first of observed observables
  observable_li_t  last_observed;  // helper for faster addition
  void             **args;         // array of pointers to values of observeds
  int              level;          // the level in the dependecy graph
  observable_li_t  observers;      // first of observers
  observable_li_t  last_observer;  // helper for faster addition
  observable_t     parent;         // helper field for storing creating parent
  fragment_t       next_fragment;  // next fragment to be activated
} observable;

enum code {
  AWAIT
};

// fragments are (in a away) observables waiting to be come active. observables
// observe other observables, so a fragment contains information about this
// other observable and an indication of which code is needed to activate it.
struct fragment {
  observable_t observed;
  enum code    statement;
};

// constructors

observable_t _new() {
  observable_t observable   = malloc(sizeof(struct observable));
  observable->prop          = UNKNOWN;
  observable->value         = NULL;
  observable->adapter       = NULL;
  observable->observeds     = NULL;
  observable->last_observed = NULL;
  observable->args          = NULL;
  observable->level         = 0;
  observable->observers     = NULL;
  observable->last_observer = NULL;
  observable->parent        = NULL;
  observable->next_fragment = NULL;
  return observable;
}

// an ExternalValueObservable simply stores a pointer to some value in memory,
// which is managed externally. when this value is updated, the observer_update
// function should be called to activate the reactive behaviour associated with
// it through this observable.
observable_t observable_from_value(void* value) {
  observable_t observable   = _new();
  observable->prop          = VALUE;
  observable->value         = value;
  return observable;
}

// an ObservingObservable wraps and observer. every observer of observables is
// inherently also an observable, due to the reactive nature of the underlying
// data(stream). the function is an adapter that transforms the values of its
// observed observables into its own current value. this value is externally
// defined and its size should therefore be provided to allow memory allocation.
observable_t observable_from_observer(observer_t observer, int size) {
  observable_t observable   = _new();
  observable->prop          = OBSERVER;
  observable->value         = (void*)malloc(size);
  observable->adapter       = observer;
  return observable;
}

// private functionality

// recompute the level for this observable. do this by computing the maximum
// level for all observed observables + 1
void _update_level(observable_t this) {
  observable_li_t observed = this->observeds;
  int level = 0;
  while(observed) {
    if(observed->ob->level > level) { level = observed->ob->level; }
    observed = observed->next;
  }
  this->level = level + 1;
}

// recompute the arguments pointer list for this observable. do this by storing
// all value pointers of our own observed observables in an array.
void _update_args(observable_t this) {
  // compute number of observed observables
  observable_li_t observed = this->observeds;
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
  observed = this->observeds;
  count = 0;
  while(observed) {
    this->args[count] = observed->ob->value;
    count++;
    observed = observed->next;
  }
}

void _add_observer(observable_t this, observable_t observer) {
  if(this->last_observer == NULL) {
    // first observer
    this->observers     = malloc(sizeof(struct observable_li));
    this->last_observer = this->observers;
  } else {
    // add one
    this->last_observer->next = malloc(sizeof(struct observable_li));
    this->last_observer = this->last_observer->next;
  }
  // fill info
  this->last_observer->ob   = observer;
  this->last_observer->next = NULL;
}

void _unlink_observer(observable_t this, observable_t observer) {
  if(this->observers == NULL) { return; } // no observers
  if(this->observers->ob == observer) {
    // remove first observer
    observable_li_t temp = this->observers;
    this->observers = this->observers->next;
    free(temp);
    return;
  }
  // find in linked list
  observable_li_t iter = this->observers;
  while(iter->next && iter->next->ob != observer) {
    iter = iter->next;
  }
  if(iter->next) {
    observable_li_t temp = iter->next;
    iter->next = iter->next->next;
    free(temp);
  }
  // update cached arg pointers and the level in the dependency graph
  _update_args(observer);
  _update_level(observer);
}

void _add_observed(observable_t this, observable_t observed) {
  if(this->last_observed == NULL) {
    // first observed
    this->observeds     = malloc(sizeof(struct observable_li));
    this->last_observed = this->observeds;
  } else {
    // add one
    this->last_observed->next = malloc(sizeof(struct observable_li));
    this->last_observed = this->last_observed->next;
  }
  // fill info
  this->last_observed->ob   = observed;
  this->last_observed->next = NULL;

  // update cached arg pointers and the level in the dependency graph
  _update_args(this);
  _update_level(this);
}

void _unlink_observed(observable_t this, observable_t observed) {
  if(this->observeds == NULL) { return; } // no observeds
  if(this->observeds->ob == observed) {
    // remove first observed
    observable_li_t temp = this->observers;
    this->observeds = this->observeds->next;
    free(temp);
    return;
  }
  // find in linked list
  observable_li_t iter = this->observeds;
  while(iter->next && iter->next->ob != observed) {
    iter = iter->next;
  }
  if(iter->next) {
    observable_li_t temp = iter->next;
    iter->next = iter->next->next;
    free(temp);
  }
  // update cached arg pointers and the level in the dependency graph
  _update_args(this);
  _update_level(this);
}

// remove all links to observers
void _unlink_all_observers(observable_t this) {
  while(this->observers) {
    // back-link from observer
    _unlink_observed(this->observers->ob, this);
    observable_li_t temp = this->observers;
    this->observers = this->observers->next;
    free(temp);
  }
}

// remove all links to observed observables
void _unlink_all_observeds(observable_t this) {
  while(this->observeds) {
    // back-link from observed
    _unlink_observer(this->observeds->ob, this);
    observable_li_t temp = this->observeds;
    this->observeds = this->observeds->next;
    free(temp);
  }
}

// actually free the entire observable structure
void _free(observable_t this) {
  _unlink_all_observers(this);
  _unlink_all_observeds(this);
  if(this->args)  { free(this->args);  }
  if(this->value) { free(this->value); }
  free(this);
}

// check all observers and remove those that are marked for disposal
void _gc(observable_t this) {
  if(this->observers == NULL) { return; }

  // garbage collect disposed observers
  if(this->observers->ob->prop & DISPOSED) {    // first observer got disposed
    observable_li_t temp = this->observers;
    this->observers = this->observers->next;
    _free(temp->ob);
    free(temp);
    return;
  }
  observable_li_t observer = this->observers;
  while(observer->next) {
    while(observer->next && (observer->next->ob->prop & DISPOSED)) {
      observable_li_t temp = observer->next;
      observer->next = observer->next->next;
      _free(temp->ob);
      free(temp);
    }
    observer = observer->next;
  }
}

// internal observer function to merge value updates to multiple observables
// into one "merged" observable observer.
void _merge(void **args, void* self) {
  observable_t merged = ((observable_t)self)->parent;
  merged->value = args[0];

  // cached args are out of date, because we're modifying the pointer itself
  // force refresh cache on all observers of merged_ob
  observable_li_t iter = merged->observers;
  while(iter) {
    _update_args(iter->ob);
    iter = iter->next;
  }

  observe_update(merged);
}

// forward declaration
observable_t _activate(fragment_t);

void _next(observable_t script) {
  if(script->next_fragment == NULL) { return; } // end of script

  // activate
  observable_t step = _activate(script->next_fragment);
  step->parent = script;

  // observe the new step
  _add_observed(script, step);

  // no next step (TODO: advance linked list)
  script->next_fragment = NULL;
}

void _finalize_await(void **args, void *this) {
  fprintf( stderr, "finalizing await\n" ); // for demo purposes ;-)
  // dispose ourselves
  dispose((observable_t)this);

  // tell the script to move to the next step
  _next(((observable_t)this)->parent);
}

observable_t _activate(fragment_t f) {
  observable_t ob = NULL;
  switch(f->statement) {
    case AWAIT:
      ob = observe(all(1, f->observed), _finalize_await, 0);
      ob->prop |= OUT_IS_SELF;
      free(f); // once activated this is no longer needed
  }
  return ob;
}

// public interface

// turns a variadic list of observables into an linked list. this is a helper
// function to allow calling observe(all(...) and pass a variable list of
// observables)
observable_li_t all(int count, ...) {
  va_list         ap;
  observable_li_t list; // first observable in the list
  observable_li_t last; // track last observable in the list for addition
  
  // import other observables
  va_start(ap, count);
  for(int i=0; i<count; i++) {
    if(i==0) {
      list = malloc(sizeof(observable_li_t));
      last = list;
    } else {
      last->next = malloc(sizeof(observable_li_t));
      last = last->next;
    }
    last->ob   = va_arg(ap, observable_t);
  }
  va_end(ap);
  
  // return casted version
  return list;
}

// start observing observed observables using an observer (function), storing
// the resulting value in a memory location of size.
observable_t observe(observable_li_t observeds, observer_t observer, int size) {
  // step 1: turn the observer into an observable
  observable_t observable_observer = observable_from_observer(observer, size);

  // step 2: add observeds and update the arguments list and our level in the
  // dependency graph
  observable_observer->observeds = observeds;
  _update_args(observable_observer);
  _update_level(observable_observer);

  // step 3: add the observer to all observeds
  while(observeds) {
    _add_observer(observeds->ob, observable_observer);
    observeds = observeds->next;
  }

  return observable_observer;
}

// marks an observable for disposing, which is honored when an update-push is
// executed on it.
void dispose(observable_t this) {
  this->prop |= DISPOSED;
}

// trigger for (external) update of observable
void observe_update(observable_t this) {
  // if we're marked for disposal (externally), we don't perform any processing
  // one of our observed parents will garbage collect us sometime
  if(this->prop & DISPOSED) { return; }

  // if we have a adapter execute it
  if(this->adapter != NULL) {
    // if the adapter is the internal _merge, we pass the parent, not the value
    if(this->prop & OUT_IS_SELF) {
      this->adapter(this->args, this);
    } else {
      this->adapter(this->args, this->value);
    }
  }

  // the logic in the adapter might have marked us for disposal, still we allow
  // the event flow to continue down to our observers one more time.

  // notify all our observers to do the same, but only those that are directly
  // dependant on us, so with level = our level + 1. those with even higher
  // levels are dependant on other levels below us and will be triggered as
  // soon as their parents all have been updated
  observable_li_t observer = this->observers;
  while(observer) {
    if(observer->ob->level == this->level + 1) {
      observe_update(observer->ob);
    }
    observer = observer->next;
  }

  // perform garbage collection on our observers
  _gc(this);
}

// extract current value from this observable
void *observable_value(observable_t observable) {
  return observable->value;
}

// merging support

// create a single observable observer from a list of observed observables.
observable_t merge(observable_li_t observed) {
  observable_t merged = observable_from_value(NULL);
  while(observed) {
    observable_t tmp = observe(all(1, observed->ob), _merge, 0);
    tmp->prop |= OUT_IS_SELF;
    tmp->parent = merged;
    observed = observed->next;
  }
  return merged;
}

// some API example functions

observable_t map(observable_t observed, observer_t adapter, int size) {
  return observe(all(1, observed), adapter, size);
}

void _addi(void **args, void *out) {
  (*(int*)out) = (*(int*)(args[0])) + (*(int*)(args[1]));
}

void _addd(void **args, void *out) {
  (*(double*)out) = (*(double*)(args[0])) + (*(double*)(args[1]));
}

observable_t addi(observable_t a, observable_t b) {
  return observe(all(2, a, b), _addi, sizeof(int));
}

observable_t addd(observable_t a, observable_t b) {
  return observe(all(2, a, b), _addd, sizeof(double));
}

// scripting support

// await fragment constructor
fragment_t await(observable_t observed) {
  fragment_t f = malloc(sizeof(struct fragment));
  f->statement = AWAIT;
  f->observed = observed;
  return f;
}

observable_t observable_from_script(fragment_t f1, fragment_t f2) {
  // activate first step
  observable_t step1 = _activate(f1);
  observable_t script = observe(all(1, step1), NULL, 0);
  step1->parent = script;
  script->next_fragment = f2;
  // TODO: VARARGS -> linked list of fragments
  return script;
}
