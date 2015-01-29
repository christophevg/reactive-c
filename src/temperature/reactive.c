// reactive c

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
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

observables_t _new_observables_list() {
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

// an observable has properties that are used internally to manage its
// functionality and/or lifetime. an observable contains a pointer to a value
// (for ExternalValueObservers). alternatively it contains a pointer to an
// process observer (function), which takes the current values of its observed
// observables and computes it own new value. these observed observables are
// stored in a linked list. because the current values of these observables as
// pointers, we can point to them too from an array. this array is a cached copy
// of the arguments passed to the process function, which now only needs to be
// computed once (when adding observed observables, through the observer()
// function). given the observed observables, we can compute the level within
// the dependency graph, being the max of the levels of the observed observable
// + 1. changes to this value need to trigger an update with all observers of an
// observable. a list of observing observables tracking this observable are also
// stored in a linked list. because observables can be created by means of a
// script, a link to the next fragement in the script is also stored in an
// observable.
typedef struct observable {
  int            prop;           // internal properties
  void           *value;         // cached or pointer to observed value <---+
  observer_t     process;        // function that given input produces _____|
  observables_t  observeds;      // first of observed observables
  void           **args;         // array of pointers to values of observeds
  int            level;          // the level in the dependecy graph
  observables_t  observers;      // first of observers
  observable_t   parent;         // helper field for storing creating parent
  fragment_t     next_fragment;  // next fragment to be activated
} observable;

enum code {
  AWAIT
};

// fragments are (in a away) observables waiting to be come active. observables
// observe other observables, so a fragment contains information about this
// other observable and an indication of which code is needed to activate it. a
// script consists of a list of fragments, so the next fragment can be retrieved
// following the next pointer.
struct fragment {
  observable_t observed;
  enum code    statement;
  fragment_t   next;
};

// constructors

observable_t _new() {
  observable_t observable   = malloc(sizeof(struct observable));
  observable->prop          = UNKNOWN;
  observable->value         = NULL;
  observable->process       = NULL;
  observable->observeds     = _new_observables_list();
  observable->args          = NULL;
  observable->level         = 0;
  observable->observers     = _new_observables_list();
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
// data(stream). the function is an process that transforms the values of its
// observed observables into its own current value. this value is externally
// defined and its size should therefore be provided to allow memory allocation.
observable_t observable_from_callback(observer_t observer, int size) {
  observable_t observable   = _new();
  observable->prop          = OBSERVER;
  observable->value         = (void*)malloc(size);
  observable->process       = observer;
  return observable;
}

// private functionality

// recompute the level for this observable. do this by computing the maximum
// level for all observed observables + 1
void _update_level(observable_t this) {
  observable_li_t observed = this->observeds->first;
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
}

// remove all links to observers
void _clear_observers(observable_t this) {
  // remove back-links from our observers
  for(observable_li_t item = this->observers->first; item; item = item->next) {
    _remove_observable(item->ob->observeds, this);
  }
  _clear_observables(this->observers);
}

// remove all links to observed observables
void _clear_observeds(observable_t this) {
  // remove back-links from our observeds
  for(observable_li_t item = this->observeds->first; item; item = item->next) {
    _remove_observable(item->ob->observers, this);
  }
  _clear_observables(this->observeds);
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
void _gc(observable_t this) {
  if(this->observers->first == NULL) {
    assert(this->observers->last == NULL);
    return;
  }

  // garbage collect disposed observers
  observable_li_t item = this->observers->first;
  if(item->ob->prop & DISPOSED) { // first got disposed
    _free(item->ob);  // free entire observable (including all links)
    return;
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
}

// internal observer function to merge value updates to multiple observables
// into one "merged" observable observer.
void _merge(void **args, void* self) {
  observable_t merged = ((observable_t)self)->parent;
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

// forward declaration for circular dependency:
// _activate -> _finalize_await -> _next -> _activate
observable_t _activate(fragment_t);

void _next(observable_t script) {
  fragment_t fragment = script->next_fragment;
  if(fragment == NULL) { return; } // end of script

  // prepare for next fragment (before _activate because it frees the fragment)
  script->next_fragment = fragment->next;

  // activate
  observable_t step = _activate(fragment);
  step->parent = script;

  // observe the new step
  _add_observable(script->observeds, step);
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
      ob = observe(all(f->observed), _finalize_await, 0);
      ob->prop |= OUT_IS_SELF;
      free(f); // once activated this is no longer needed
  }
  return ob;
}

// public interface

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

// start observing observed observables using an observer (function), storing
// the resulting value in a memory location of size.
observable_t observe(observables_t observeds, observer_t callback, int size) {
  // step 1: turn the observer into an observable
  observable_t observer = observable_from_callback(callback, size);

  // step 2: add observeds and update the arguments list and our level in the
  // dependency graph
  observer->observeds = observeds;
  _update_args(observer);
  _update_level(observer);

  // step 3: add a back-link to the observer to all observeds
  observable_li_t iter = observeds->first;
  while(iter) {
    _add_observable(iter->ob->observers, observer);
    iter = iter->next;
  }
  return observer;
}

// marks an observable for disposing, which is honored when an update-push is
// executed on it.
void dispose(observable_t this) {
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
observable_t merge(observables_t observeds) {
  observable_t merged = observable_from_value(NULL);
  observable_li_t observed = observeds->first;
  while(observed) {
    observable_t tmp = observe(all(observed->ob), _merge, 0);
    tmp->prop |= OUT_IS_SELF;
    tmp->parent = merged;
    observed = observed->next;
  }
  return merged;
}

// some API example functions

observable_t map(observable_t observed, observer_t process, int size) {
  return observe(all(observed), process, size);
}

void _addi(void **args, void *out) {
  (*(int*)out) = (*(int*)(args[0])) + (*(int*)(args[1]));
}

void _addd(void **args, void *out) {
  (*(double*)out) = (*(double*)(args[0])) + (*(double*)(args[1]));
}

observable_t addi(observable_t a, observable_t b) {
  return observe(all(a, b), _addi, sizeof(int));
}

observable_t addd(observable_t a, observable_t b) {
  return observe(all(a, b), _addd, sizeof(double));
}

// scripting support

// await fragment constructor
fragment_t await(observable_t observed) {
  fragment_t f = malloc(sizeof(struct fragment));
  f->statement = AWAIT;
  f->observed = observed;
  return f;
}

observable_t __observable_from_script(int count, ...) {
  if(count<1) { return NULL; }
  
  observable_t script = _new();                   // no value/adapter (for now)
   
  va_list ap;
  va_start(ap, count);
  script->next_fragment = va_arg(ap, fragment_t); // first due to count >= 1
  fragment_t fragments = script->next_fragment;
  for(int i=1; i<count; i++) {                    // remaining fragments
    fragments->next = va_arg(ap, fragment_t);
    fragments = fragments->next;
  }
  va_end(ap);

  _next(script);  // activates the first fragment in a consistent way

  return script;
}
