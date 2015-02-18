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

// a list of script waiting to proceed
struct observables scripts_waiting = { NULL, NULL };

// constructor and accessors for an observables list
observables_t _new_observables_list(void) {
  observables_t list = malloc(sizeof(struct observables));
  list->first = NULL;
  list->last = NULL;
  return list;
}

#define _is_empty(list) list->first == NULL

void _add_observable(observables_t list, observable_t observable) {
  if(_is_empty(list)) {
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

bool _contains_observable(observables_t list, observable_t observable) {
  observable_li_t item = list->first;
  while(item) {
    if(item->ob == observable) { return true; }
    item = item->next;
  }
  return false;
}

void _remove_observable(observables_t list, observable_t observable) {
  if(_is_empty(list)) {
    assert(list->last == NULL);
    return; // empty list
  }
  observable_li_t head = list->first;
  if(head->ob == observable) {
    // first in the list, remove the head
    list->first = head->next;
    free(head);
    if(_is_empty(list)) { list->last = NULL; } // it was also the only item
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
  while(list->first) {
    observable_li_t item = list->first;
    list->first = list->first->next;
    free(item);
  }
  list->last = list->first;
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
  UNKNOWN     =   0,
  VALUE       =   1,
  OBSERVER    =   2,
  OUT_IS_PART =   4, // don't provide value but participants (internal use)
  DISPOSED    =   8, // used to mark an observable as ready to be removed
  SUSPENDED   =  16,
  DELAYED     =  32,
  STOP_PROP   =  64,
  EXPORTED    = 128,
  OBSERVED    = 256,
};

// macro's to hide underlying bitwise operations (hey, I like "readable code")
#define _use_participants(o)  (o->prop |=  OUT_IS_PART)
#define _uses_participants(o) (o->prop &   OUT_IS_PART)
#define _is_disposed(o)       (o->prop &   DISPOSED)
#define _is_suspended(o)      (o->prop &   SUSPENDED)
#define _is_delayed(o)        (o->prop &   DELAYED)
#define _mark_observed(o)     (o->prop |=  OBSERVED)
#define _propagate(o)         (o->prop &= ~STOP_PROP)
#define _is_propagating(o)  (!(o->prop &   STOP_PROP))
#define _dont_propagate(o)    (o->prop |=  STOP_PROP)
#define _mark_exported(o)     (o->prop |=  EXPORTED)
#define _is_exported(o)       (o->prop &   EXPORTED)
#define _is_observed(o)       (o->prop &   OBSERVED)

// structure of observable is private
typedef struct observable {
  char           *label;         // textual representation
  int            prop;           // internal properties
  unknown_t      value;          // cached or pointer to observed value <---+
  int            type_size;      // sizeof(typeof(value))
  observer_t     process;        // function that given input produces _____|
  observables_t  observeds;      // first of observed observables
  observables_t  observers;      // first of observers
  unknown_t      *args;          // cached array of ptrs to values of observeds
  int            level;          // the level in the dependecy graph
  validator_t    validate;       // validate if an update is propagated
  observable_t   parent;         // functional ref to "creating" observable
  observable_t   next;           // functional ref to "next" observable
  // events
  observable_callback_t  on_dispose;
  observable_callback_t  on_activation;
} observable;

// private functionality

// debugging functionality
#ifndef NDEBUG
void _debug_level(char* title, observable_t this, int level) {
  printf("%*s: %s (%p) prop:", level, title, this->label, (void*)this);
  if(_uses_participants(this)) { printf(" participants"); }
  if(_is_disposed(this))       { printf(" disposed"); }
  if(_is_suspended(this))      { printf(" suspended"); }
  if(_is_delayed(this))        { printf(" delayed"); }
  if(!_is_propagating(this))   { printf(" no_propagation"); }
  printf("\n");
  if(this->observeds->first) {
    printf("%*.s   observing:\n", level, "");
    foreach(observable, this->observeds, {
      _debug_level("   - ", iter->current->ob, level+3);
    });
  }
  if(this->next && this->parent == NULL) {
    printf("steps:\n");
    observable_t step = this->next;
    int c = 1;
    while(step) {
      printf("%d)\n", c++);
      _debug_level("", step, level + 3);
      step = step->next;
    }
  }
}
#define _debug(t,o) _debug_level(t,o,0)
#else
#define _debug(t,o)
#endif

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
  // optionally free existing list
  if(this->args) { free(this->args); }
  
  // allocate memory to hold cached list of pointers to observed values
  this->args = malloc(sizeof(unknown_t) * _count_observables(this->observeds));
  
  // copy pointers to values
  int i = 0;
  foreach(observable, this->observeds, {
    this->args[i++] = iter->current->ob->value;
  });

  return this;
}

// remove all links to observers
observable_t _clear_observers(observable_t this) {
  // remove back-links from our observers
  foreach(observable, this->observers, {
    _remove_observable(iter->current->ob->observeds, this);
  });
  _clear_observables(this->observers);
  _debug("CLEARED OBSERVERS", this);
  return this;
}

// remove all links to observed observables
observable_t _clear_observeds(observable_t this) {
  // remove back-links from our observeds
  foreach(observable, this->observeds, {
    _remove_observable(iter->current->ob->observers, this);
  });
  _clear_observables(this->observeds);
  _debug("CLEARED OBSERVEDS", this);
  return this;
}

// actually free the entire observable structure
void _free(observable_t this) {
  _debug("FREEING", this);
  _clear_observers(this);
  _clear_observeds(this);
  if(this->args)  { free(this->args);  this->args  = NULL; }
  if(this->value) { free(this->value); this->value = NULL; }
  free(this);
}

observable_t bin = NULL;

#define empty_bin() (bin == NULL)

void _trash(observable_t observable) {
  observable->next = bin;
  bin = observable;
}

void _empty_trash() {
  while(bin) {
    observable_t trash = bin;
    bin = bin->next;
    _free(trash);
  }
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
  if(_is_delayed(this)) { return; }
  // make sure we propagate once we're active
  _propagate(this);

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
  if(_is_delayed(this)) { return; }

  observable_t source = (observable_t)((participants_t)ob->observer)->source;

  // track updates by marking their link as observed
  int count   = 0;
  int observed = 0;
  foreach(observable, this->observeds, {
    if(iter->current->ob == source) {
      _debug("ALL OBSERVED UPDATE", source);
      _mark_observed(iter->current);
    }
    // keep stats
    count++;
    if(_is_observed(iter->current)) { observed++; }
  });
    
  // have we observed all items emit updates?
  if(observed == count) {
    _propagate(this);
    _debug("FINALIZED ALL", this);
    dispose(this);
  }
}

void _any_handler(observation_t ob) {
  observable_t this = (observable_t)((participants_t)ob->observer)->target;
  if(_is_delayed(this)) { return; }
  // as soon as we are "touched", we propagate again
  _propagate(this);

  _debug("FINALIZED ANY", this);

  // any hit is ok, we dispose ourselves and will be cleaned up by one of our
  // observeds when its done with us. in the meantime, we'll ignore more updates
  dispose(this);
}

observable_t _proceed(observable_t script) {
  // add the script to the list of script that is ready to proceed, it will be
  // moved ahead once the top-level observation has been processed.
  // TODO: is this fine-grained enough? come up with scenario to invalidate this
  if(!_contains_observable(&scripts_waiting, script)) {
    _add_observable(&scripts_waiting, script);
  }
  return script;
}

observable_t _step(observable_t script) {
  observable_t step = script->next;
  if(step == NULL) { return NULL; } // end of script
  _debug("STEP TO", step);
  
  // link new step to script
  step->parent = script->parent ? script->parent : script;
  
  // prepare for next step
  script->next = step->next;

  // start
  start(step);
  // is the parent/script has an on_activation handler, execute it on the step
  if(step->parent && step->parent->on_activation) {
    step->parent->on_activation(step);
  }

  // do we (still) have anything to do? our observeds might already be done.
  while(step && step->observeds && _count_observables(step->observeds) == 0) {
    dispose(step);
    step = _step(step);
  }

  return step;
}

void _finalize_await(observation_t ob) {
  observable_t this = (observable_t)((participants_t)ob->observer)->target;
  _debug("FINALIZE AWAIT", this);

  if(_is_suspended(this)) { return; }

  // dispose ourselves
  dispose(this);

  // tell the script to move to the next step
  _proceed(this->parent);
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
  return this;
}

// create an observable observer (function), storing the resulting value in a
// memory location of size.
observable_t __observing(char *label, observables_t observeds,
                         observer_t observer, int size)
{
  // step 1: turn the observer into an observable
  observable_t this = _new(label);
  this->prop        = OBSERVER;
  this->value       = size ? (unknown_t)malloc(size) : NULL;
  this->type_size   = size;
  this->process     = observer;
  this->observeds   = observeds;    // this is already partial in the graph
                                    // but cannot be used, no back-links

  // step 2: update the arguments list and our level in the dependency graph
  _update_args(this);
  _update_level(this);

  // step 3: add a back-link to the observer to all observeds
  foreach(observable, this->observeds, {
    _add_observable(iter->current->ob->observers, this);
  });

  return this;
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
  foreach(observable, this->observeds, {
    undelay(iter->current->ob);
  });
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
  if(this->validate){
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
    struct observation ob = { .observeds = this->args };
    if(_uses_participants(this)) {
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
  if(_is_propagating(this)) {
    // notify all our observers to do the same, but only those that are directly
    // dependant on us, so with level = our level + 1. those with even higher
    // levels are dependant on other levels below us and will be triggered as
    // soon as their parents all have been updated
    foreach(observable, this->observers, {
      if( ! _is_disposed(iter->current) ) {
        if(iter->current->ob->level == this->level + 1) {
          _observe_update(iter->current->ob, this);
        }
      }
    });
  }
}

// public method can only trigger update, not be a source
void observe_update(observable_t observable) {
  _debug("UPDATE", observable);
  _observe_update(observable, NULL);
  // During the propagation of the update, scripts might have finalized a step,
  // and are waiting to proceed. This cannot be done safely otherwise (for now)
  // because an update triggering a step in the script might also trigger the
  // next step, which might not what is intended.
  // At the end of an observation propagation, we proceed all scripts that are
  // marked as such.
  foreach(observable, &scripts_waiting, {
    _step(iter->current->ob);
  });
  _clear_observables(&scripts_waiting);
  // empty the trash
  _empty_trash();
}

// extract current value from this observable
unknown_t observable_value(observable_t observable) {
  return observable->value;
}

// generic constructor for observables that observe a set of observables
observable_t __combine(char *label, observables_t observeds, observer_t handler) {
  observable_t combination = start(__observing(label, observeds, handler, 0));
  _use_participants(combination);
  return combination;
}

// create a single observable observer from a list of observed observables.
observable_t __merge(char *label, observables_t observeds) {
  observable_t observer = __combine(label, observeds, _merge_handler);
  _debug("MERGE", observer);
  return observer;
}  

observable_t __all(char *label, observables_t observeds) {
  observable_t observer = __combine(label, observeds, _all_handler);
  _debug("ALL", observer);
  return observer;
}

observable_t __any(char *label, observables_t observeds) {
  observable_t observer = __combine(label, observeds, _any_handler);
  _debug("ANY", observer);
  return observer;
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
  _use_participants(filter);
  filter->validate = validator;
  return filter;
}

// scripting support

// constructor for observer that wait until another observer emits
observable_t await(observable_t observable) {
  observable_t this = __observing("await", just(observable), _finalize_await, 0);
  _use_participants(this);
  _debug("AWAIT", this);
  return suspended(this);
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

  _debug("SCRIPT", script);

  return script;
}

observable_t run(observable_t script) {
  return _step(script);  // activates the first step in a consistent way
}

// output support

void __to_dot(observable_t this, FILE *fp, bool show_memory, bool preamble) {
  if(_is_exported(this)) { return; }
  _mark_exported(this);

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
    if(!_is_exported(iter->current)) {
      _mark_exported(iter->current);
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
