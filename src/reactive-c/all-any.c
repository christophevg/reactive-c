#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "iterator.h"

#include "debug.h"

void _all_handler(observation_t ob) {
  if(_is_delayed(ob->self)) { return; }

  // track updates by marking their link as observed
  int count   = 0;
  int observed = 0;
  foreach(observable_li_t, iter, ob->self->observeds) {
    if(iter->ob == ob->source) {
      _debug("ALL OBSERVED UPDATE", ob->source);
      _mark_observed(iter);
    }
    // keep stats
    count++;
    if(_is_observed(iter)) { observed++; }
  }
    
  // have we observed all items emit updates?
  if(observed == count) {
    _propagate(ob->self);
    _debug("FINALIZED ALL", ob->self);
    dispose(ob->self);
  }
}

observable_t __all(char *label, observables_t observeds) {
  observable_t observer = _combine(label, observeds, _all_handler);
  _debug("ALL", observer);
  return observer;
}

void _any_handler(observation_t ob) {
  if(_is_delayed(ob->self)) { return; }
  // as soon as we are "touched", we propagate again
  _propagate(ob->self);

  _debug("FINALIZED ANY", ob->self);

  // any hit is ok, we dispose ourselves and will be cleaned up by one of our
  // observeds when its done with us. in the meantime, we'll ignore more updates
  dispose(ob->self);
}

observable_t __any(char *label, observables_t observeds) {
  observable_t observer = _combine(label, observeds, _any_handler);
  _debug("ANY", observer);
  return observer;
}
