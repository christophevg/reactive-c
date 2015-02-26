#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "iterator.h"

#include "debug.h"

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
  while(step && step->observeds && observables_count(step->observeds) == 0) {
    dispose(step);
    step = _step(script);
  }

  return step;
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

// a list of script waiting to proceed
struct observables _scripts_waiting = { NULL, NULL };
observables_t scripts_waiting = &_scripts_waiting;

void _proceed_scripts_waiting(observable_t _) {
  // During the propagation of the update, scripts might have finalized a step,
  // and are waiting to proceed. This cannot be done safely otherwise (for now)
  // because an update triggering a step in the script might also trigger the
  // next step, which might not what is intended.
  // At the end of an observation propagation, we proceed all scripts that are
  // marked as such.
  foreach(observable_li_t, iter, scripts_waiting) {
    _step(iter->ob);
  }
  observables_clear(scripts_waiting);
}

observable_t _proceed(observable_t script) {
  // make sure that on the next observation, our handler is registered... ONCE
  static bool registered = false;
  if(!registered) {
    _on_update(_proceed_scripts_waiting);
    registered = true;
  }

  // add the script to the list of script that is ready to proceed, it will be
  // moved ahead once the top-level observation has been processed.
  // TODO: is this fine-grained enough? come up with scenario to invalidate this
  if(!observables_contains(scripts_waiting, script)) {
    observables_add(scripts_waiting, script);
  }
  return script;
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

// constructor for observer that wait until another observer emits
observable_t await(observable_t observable) {
  observable_t this = __observing("await", just(observable), _finalize_await, 0);
  _use_participants(this);
  _debug("AWAIT", this);
  return suspended(this);
}

