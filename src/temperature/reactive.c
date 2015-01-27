// reactive c

#include <stdlib.h>
#include <stdarg.h>

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
  VALUE       = 1,
  OBSERVER    = 3,
  OUT_IS_SELF = 4 // don't provide value but the entire observable as out param
};

// an observable contains a pointer to a value (for ExternalValueObservers).
// alternatively it contains a pointer to an adapter observer (function), which
// takes the current values of its observed observables and computes it own new
// value. these observed observables are stored in a linked list. because the
// current values of these observables as pointers, we can point to them too
// from an array. this array is a cached copy of the arguments passed to the
// adapter function, which now only needs to be computed once (when adding
// observed observables, through the observer() function). given the observed
// observables, we can compute the level within the dependency graph, being the
// max of the levels of the observed observable + 1. changes to this value need
// to trigger an update with all observers of an observable. finally, the list
// of observing observables tracking this observable are also stored in a linked
// list.
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
} observable;

// constructors

// an ExternalValueObservable simply stores a pointer to some value in memory,
// which is managed externally. when this value is updated, the observer_update
// function should be called to activate the reactive behaviour associated with
// it through this observable.
observable_t observable_from_value(void* value) {
  observable_t observable   = malloc(sizeof(struct observable));
  observable->prop          = VALUE;
  observable->value         = value;
  observable->adapter       = NULL;
  observable->observeds     = NULL;
  observable->last_observed = NULL;
  observable->args          = NULL;
  observable->level         = 0;
  observable->observers     = NULL;
  observable->last_observer = NULL;
  observable->parent        = NULL;
  return observable;
}

// an ObservingObservable wraps and observer. every observer of observables is
// inherently also an observable, due to the reactive nature of the underlying
// data(stream). the function is an adapter that transforms the values of its
// observed observables into its own current value. this value is externally
// defined and its size should therefore be provided to allow memory allocation.
observable_t observable_from_observer(observer_t observer, int size) {
  observable_t observable   = malloc(sizeof(struct observable));
  observable->prop          = OBSERVER;
  observable->value         = (void*)malloc(size);
  observable->adapter       = observer;
  observable->observeds     = NULL;
  observable->last_observed = NULL;
  observable->args          = NULL;
  observable->level         = 0;
  observable->observers     = NULL;
  observable->last_observer = NULL;
  observable->parent        = NULL;
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
  
  // allocate memory
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

void _remove_observer(observable_t this, observable_t observer) {
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

// cleanly frees the entire observable/observer
bool dispose(observable_t observer) {
  // check that we can be disposed of = no one is observing us
  if(observer->observers != NULL) {
    return false;
  }

  // step 1: remove observer from all observeds
  int count = 0;
  while(observer->observeds) {
    count++;
    _remove_observer(observer->observeds->ob, observer);
    observable_li_t temp = observer->observeds;
    observer->observeds = observer->observeds->next;
    free(temp);
  }

  // step 2: free cached array of pointers to values of observers
  free(observer->args);

  // step 3: (optionally) free local value
  if(observer->value) { free(observer->value); }

  // step 4: release the observer itself
  free(observer);

  return true;
}

// trigger for (external) update of observable
void observe_update(observable_t this) {
  // if we have a adapter execute it
  if(this->adapter != NULL) {
    // if the adapter is the internal _merge, we pass the parent, not the value
    if(this->prop & OUT_IS_SELF) {
      this->adapter(this->args, this);
    } else {
      this->adapter(this->args, this->value);
    }
  }

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
}

// extract current value from this observable
void *observable_value(observable_t observable) {
  return observable->value;
}

// merging support

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
