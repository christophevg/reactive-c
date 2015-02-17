// reactive c

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "reactive.h"

#include "iterator.h"

// observation internal extension
// alternative:
// 2. a pointer to a participants structure, containing pointers to the emitter
//    and receiver (internal use only)
typedef struct participants {
  observable_t source;
  observable_t target;
} *participants_t;

// linked list item (LI) for observables. "next" can't be added to observable
// because each observable can be grouped with other observers or observed
// observables.
typedef struct observable_li {
  observable_t         ob;
  int prop;
  struct observable_li *next;
} *observable_li_t;

// the list is also packed in a struct, containing the root/first list item and
// a reference to the last, for faster additions.
struct observables {
  observable_li_t first;
  observable_li_t last;
} observables;

construct_iterator(observable)

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
    list->first->prop = 0;
    list->first->ob   = NULL;
    list->first->next = NULL;
    list->last = list->first;
  } else {
    // prepare next list item
    list->last->next = malloc(sizeof(struct observable_li));
    list->last->next->prop = 0;
    list->last->next->ob   = NULL;
    list->last->next->next = NULL;
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

bool _no_more_observables(observables_t list) {
  return list->first == NULL;
}

int _count_observables(observables_t list) {
  int count = 0;
  foreach(observable, list, {
    count++;
  });
  return count;
}

// observables can have several properties, which influences the internal
// workings.
enum properties {
  UNKNOWN     =  0,
  VALUE       =  1,
  OBSERVER    =  2,
  OUT_IS_PART =  4, // don't provide value but participants (internal use)
  DISPOSED    =  8, // used to mark an observable as ready to be removed
  SUSPENDED   = 16,
  DELAYED     = 32,
  STOP_PROP   = 64,
  EXPORTED    = 128,
};

// structure of observable is private
typedef struct observable {
  char           *label;
  int            prop;           // internal properties
  unknown_t      value;          // cached or pointer to observed value <---+
  int            type_size;      // sizeof(typeof(value))
  observer_t     process;        // function that given input produces _____|
  observables_t  observeds;      // first of observed observables
  unknown_t      *args;          // array of pointers to values of observeds
  int            level;          // the level in the dependecy graph
  observables_t  observers;      // first of observers
  validator_t    validate;       // validate if an update is propagated
  // scripting support
  observable_t   parent;
  observable_t   next;           // if this observable is done, start the next
  // events
  observable_callback_t  on_dispose;
  observable_callback_t  on_activation;
} observable;

// private functionality

// constructor for empty observable
observable_t _new(char *label) {
  observable_t this   = malloc(sizeof(struct observable));
  this->label         = label;
  this->prop          = UNKNOWN;
  this->value         = NULL;
  this->type_size     = 0;
  this->process       = NULL;
  this->observeds     = _new_observables_list();
  this->args          = NULL;
  this->level         = 0;
  this->observers     = _new_observables_list();
  this->validate      = NULL;
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
  int count = 0;
  foreach(observable, this->observeds, {
    count++;
  });
  // optionally free existing list
  if(this->args != NULL) { free(this->args); }
  
  // allocate memory to hold cached list of pointers to observed values
  this->args = malloc(sizeof(unknown_t)*count);
  
  // copy pointers to values
  count = 0;
  foreach(observable, this->observeds, {
    this->args[count] = iter->current->ob->value;
    count++;
  });

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
  // do I _have_ any observers?
  if(this->observers->first == NULL) {
    assert(this->observers->last == NULL);
    return this;
  }

  // yes, check them and garbage collect disposed observers
  observable_li_t item = this->observers->first;

  // first got disposed
  if(item->ob->prop & DISPOSED) {
    _free(item->ob);  // free entire observable (including all links)
    return this;
  }
  // not first, look further
  while(item->next) {
    if(item->next->ob->prop & DISPOSED) {
      observable_li_t todo = item;
      item = item->next; // else there is no next ;-)
      _free(todo->ob);
    } else {
      item = item->next;
    }
  }
  return this;
}

// update observers' arguments - probably because this' value changed location
void _update_observers_args(observable_t this) {
  foreach(observable, this->observers, {
    _update_args(iter->current->ob);
  });
}

// internal observer function to merge value updates to multiple observables
// into one "merged" observable observer.
void _merge_handler(observation_t ob) {
  observable_t this = ((participants_t)ob->observer)->target;
  if(this->prop & SUSPENDED) { return; }
  if(this->prop & DELAYED)   { return; }

  // redirect value to the value of the emitting merged observable
  this->value = ((participants_t)ob->observer)->source->value;

  // cached args are out of date, because we're modifying the pointer itself
  // force refresh cache on all observers of merged_ob
  _update_observers_args(this);

  // in _observe_update, after the call to this handler, the observers are
  // triggered, who now will get updated args
}

void _all_handler(observation_t ob) {
  observable_t this = (observable_t)((participants_t)ob->observer)->target;
  if(this->prop & SUSPENDED) { return; }
  if(this->prop & DELAYED)   { return; }

  observable_t observed = (observable_t)((participants_t)ob->observer)->source;

  // track item by removing it from our observeds list, we're no longer 
  // interested in updates
  int count=0;
  int stopped=0;
  foreach(observable, this->observeds, {
    if(iter->current->ob == observed) {
      iter->current->prop |= DISPOSED;
    }
    count++;
    if(iter->current->prop & DISPOSED) { stopped++; }
  });
    
  // have we seen all items?
  // if(_no_more_observables(this->observeds)) {
  if(count == stopped) {
    this->prop &= ~(STOP_PROP); // allow propagation to take place
    dispose(this);
  }
}

void _any_handler(observation_t ob) {
  observable_t this = (observable_t)((participants_t)ob->observer)->target;
  if(this->prop & SUSPENDED) { return; }
  if(this->prop & DELAYED)   { return; }

  // any hit is ok, we dispose ourselves and will be cleaned up by one of our
  // observeds when its done with us. in the meantime, we'll ignore more updates
  dispose(this);
}

observable_t _step(observable_t script) {
  observable_t step = script->next;
  if(step == NULL) { return NULL; } // end of script
  
  // link new step to script
  step->parent = script->parent ? script->parent : script;
  
  // prepare for next step
  script->next = step->next;

  // start
  start(step);
  if(step->parent && step->parent->on_activation) {
    step->parent->on_activation(step);
  }

  // observe the new step (future use)
  // _add_observable(script->observeds, step);
  
  return step;
}

void _finalize_await(observation_t ob) {
  observable_t this = (observable_t)((participants_t)ob->observer)->target;

  if(this->prop & SUSPENDED) { return; }

  // dispose ourselves
  dispose(this);

  // tell the script to move to the next step
  // TODO: could this be done with an on_dispose handler ? (once multiple?) :-)
  //       this might solve the parent relation ship ;-)
  _step(this->parent);
}

// turns a variadic list of observables into an linked list. this is a helper
// function to allow calling observe(each(...) and pass a variable list of
// observables)
observables_t __each(int count, ...) {
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
observable_t __observing_value(char *label, unknown_t value, int size) {
  observable_t this = _new(label);
  this->prop        = VALUE;
  this->value       = value;
  this->type_size   = size;
  return suspended(this);
}

// create an observable observer (function), storing the resulting value in a
// memory location of size.
observable_t __observing(char *label, observables_t observeds,
                         observer_t observer, int size)
{
  // step 1: turn the observer into an observable
  observable_t this = _new(label);
  this->prop        = OBSERVER;
  this->value       = (unknown_t)malloc(size);
  this->type_size   = size;
  this->process     = observer;
  this->observeds   = observeds;    // this is already partial in the graph
                                    // but cannot be used, no back-links

  // step 2: update the arguments list and our level in the dependency graph
  _update_args(this);
  _update_level(this);

  // step 3: add a back-link to the observer to all observeds
  observable_li_t iter = this->observeds->first;
  while(iter) {
    _add_observable(iter->ob->observers, this);
    iter = iter->next;
  }

  // step 4: by default we're SUSPENDED
  return suspended(this);
}

// public interface

// actually activate the observable in the dependecy graph
observable_t start(observable_t this) {
  // step 1: acticate ourselves
  this->prop &= ~SUSPENDED;
  // step 2: if we have delayed observeds, un-delay them
  foreach(observable, this->observeds, {
    iter->current->ob->prop &= ~(DELAYED);
  });
  
  return this;
}

// undo start ;-)
observable_t suspend(observable_t this) {
  this->prop |= SUSPENDED;
  return this;
}

observable_t delay(observable_t this) {
  this->prop |= DELAYED;
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
  if(this->on_dispose) { 
    this->on_dispose(this);
  }
  this->prop |= DISPOSED;
}

// trigger for (external) update of observable
void _observe_update(observable_t this, observable_t source) {
  // if we're marked for disposal (externally), we don't perform any processing
  // one of our observed parents will garbage collect us (sometime)
  // TODO: only true for observables that observe and can be updated
  if(this->prop & DISPOSED) { return; }
  
  // if we're suspended, we ignore this update
  if(this->prop & SUSPENDED) { return; }
  
  // if we have a filter, check if we want to propagate
  if(this->validate){
    if(! this->validate(source->value) ) {
      this->prop |= STOP_PROP;
      return;
    } else {
      this->prop &= ~STOP_PROP;
    }
  }

  // if we have a process execute it (IF WE'RE NOT OUR OWN SOURCE)
  if(this->process != NULL && this != source) {
    // do we pass the value or the object itself?
    struct observation ob = { .observeds = this->args };
    if(this->prop & OUT_IS_PART) {
      struct participants participants = {.source = source, .target=this};
      ob.observer = (unknown_t)&participants;
    } else {
      ob.observer = (unknown_t)this->value;
    }
    this->process(&ob);
  }

  // the logic in the process might have marked us for disposal, still we allow
  // the event flow to continue down to our observers one more time.

  // unless we don't want to propagate an update...
  if(!(this->prop & STOP_PROP)) {
    // notify all our observers to do the same, but only those that are directly
    // dependant on us, so with level = our level + 1. those with even higher
    // levels are dependant on other levels below us and will be triggered as
    // soon as their parents all have been updated
    foreach(observable, this->observers, {
      if( ! (iter->current->prop & DISPOSED) ) {
        if(iter->current->ob->level == this->level + 1) {
          _observe_update(iter->current->ob, this);
        }
      }
    });
  }

  // perform garbage collection (on our observers)
  _gc(this);
}

// public method can only trigger update, not be a source
void observe_update(observable_t observable) {
  _observe_update(observable, NULL);
}

// extract current value from this observable
unknown_t observable_value(observable_t observable) {
  return observable->value;
}

// generic constructor for observables that observe a set of observables
observable_t __combine(char *label, observables_t obs, observer_t handler) {
  observable_t combination = start(__observing(label, obs, handler, 0));
  combination->prop |= OUT_IS_PART;
  return combination;
}

// create a single observable observer from a list of observed observables.
observable_t __merge(char *label, observables_t obs) {
  return __combine(label, obs, _merge_handler);
}  

observable_t __all(char *label, observables_t obs) {
  observable_t observer = __combine(label, obs, _all_handler);
  observer->prop |= STOP_PROP;
  return observer;
}

observable_t __any(char *label, observables_t obs) {
  return __combine(label, obs, _any_handler);
}

// fold support

observable_t __fold_int(observable_t ob, observer_t folder, int init) {
  observable_t folded = observe(just(ob), folder, sizeof(int));
  *((int*)folded->value) = init;
  return folded;
}

observable_t __fold_double(observable_t ob, observer_t folder, double init) {
  observable_t folded = observe(just(ob), folder, sizeof(double));
  *((double*)folded->value) = init;
  return folded;
}

// filter support
void _copy_value(observation_t ob) {
  observable_t this = ((participants_t)ob->observer)->target;
  // redirect value to the value of the emitting merged observable
  observable_value_copy(((participants_t)ob->observer)->source, this);

  // cached args are out of date, because we're modifying the pointer itself
  // force refresh cache on all observers of merged_ob
  _update_observers_args(this);
}

observable_t __filter(int size, observable_t observable, validator_t validator) {
  observable_t filter = observe(just(observable), _copy_value, size);
  filter->prop |= OUT_IS_PART;
  filter->validate = validator;
  return filter;
}

// scripting support

// constructor for observer that wait until another observer emits
observable_t await(observable_t observable) {
  observable_t this = __observing("await", just(observable), _finalize_await, 0);
  this->prop       |= OUT_IS_PART;
  return this;
}

// constructor for a script, consisting of <count> inactive observables
observable_t __script(int count, ...) {
  if(count<1) { return NULL; }
  
  observable_t script = _new("script");
   
  va_list ap;
  va_start(ap, count);

  // first due to count >= 1
  script->next = va_arg(ap, observable_t);
  observable_t step = script->next;
  step->parent = script;

  // remaining steps
  for(int i=1; i<count; i++) {
    step->next = va_arg(ap, observable_t);
    step = step->next;
    step->parent = script;
  }
  va_end(ap);

  return script;
}

observable_t run(observable_t script) {
  return _step(script);  // activates the first step in a consistent way
}

// output support

void __to_dot(observable_t this, FILE *fp, bool show_memory, bool preamble) {
  if(this->prop & EXPORTED) { return; }
  this->prop |= EXPORTED;

  if(preamble) {
    fprintf(fp,
      "digraph {\n"
      "  ordering=out;\n"
      "  ranksep=.4;\n"
      "  rankdir = BT;\n"
      "  node [shape=plaintext, fixedsize=true, fontsize=11, "
      "  fontname=\"Courier\", width=.25, height=.25];\n"
      "  edge [arrowsize=.6]\n"
    );
  }

  // self node
  if(show_memory) {
    fprintf(fp, "\"%p\" [label=\"%s\n%p\"", (void*)this, this->label, (void*)this);
  } else {
    fprintf(fp, "\"%p\" [label=\"%s\"", (void*)this, this->label);
  }
  // delayed/suspended observables are grey
  if(this->prop & DELAYED || this->prop & SUSPENDED) {
    fprintf(fp, " color=\"grey\", style=\"filled\"");
  }
  // values are green
  if(this->prop & VALUE) {
    fprintf(fp, " color=\"green\", style=\"filled\"");
  }
  fprintf(fp, "]\n");

  // observeds
  foreach(observable, this->observeds, {
    // only generate links for observed ... not also for observers
    if(!(iter->current->prop & EXPORTED)) {
      iter->current->prop |= EXPORTED;
      fprintf(fp, "\"%p\" -> \"%p\"\n", (void*)this, (void*)iter->current->ob);
    }
    // recurse
    __to_dot(iter->current->ob, fp, show_memory, false);
  });

  // sequential relationships (e.g. scripts' steps)
  if(this->next) {
    fprintf(fp, "\"%p\" -> \"%p\" [style=\"dotted\"]\n", (void*)this, (void*)this->next);
    fprintf(fp, "{ rank = same; \"%p\" \"%p\" }", (void*)this, (void*)this->next);
  }

  // recurse observers
  foreach(observable, this->observers, {
    __to_dot(iter->current->ob, fp, show_memory, false);
  });

  // recurse parent
  if(this->parent) {
    __to_dot(this->parent, fp, show_memory, false);
  }

  if(preamble) { fprintf(fp, "}\n"); }
}

// value manipulation support

void observable_value_copy(observable_t src, observable_t trg) {
  // we don't know what we're copying, but we know the size ;-)
  // only copy when internal memory space is the same
  assert(src->type_size == trg->type_size);
  memcpy(trg->value, src->value, src->type_size);
}

void __set_int(observable_t this, int value) {
  assert(this->type_size == sizeof(int));
  memcpy(this->value, &value, sizeof(int));
  observe_update(this);
}

void __set_double(observable_t this, double value) {
  assert(this->type_size == sizeof(double));
  memcpy(this->value, &value, sizeof(double));
  observe_update(this);
}
