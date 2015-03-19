#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "debug.h"

// internal observer function to merge value updates to multiple observables
// into one "merged" observable observer.
void _merge_handler(observation_t ob) {
  if(_is_delayed(ob->self)) { return; }
  // make sure we propagate once we're active
  _propagate(ob->self);

  // redirect value to the value of the emitting merged observable
  ob->self->value = ob->source->value;

  // cached args are out of date, because we're modifying the pointer itself
  // force refresh cache on all observers of merged_ob
  _update_observers_args(ob->self);

  // in _observe_update, after the call to this handler, the observers are
  // triggered, who now will get updated args
}

// create a single observable observer from a list of observed observables.
observable_t __merge(char *label, observables_t observeds) {
  observable_t observer = _combine(label, observeds, _merge_handler);
  _debug("MERGE", observer);
  return observer;
}  
