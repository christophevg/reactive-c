#include <stdlib.h>
#include <stdio.h>

#include "observable.h"
#include "when.h"

#include "internals.h"

void _dup(observation_t ob) {
  __set(ob->self, ob->observeds[0], ob->self->type_size);
}

observable_t when(observable_t ob, event_t ev, observable_callback_t ac) {
  observable_t watcher = __observing("watcher", just(ob), _dup, ob->type_size);
  if(ev & IS_DONE) {
    on_dispose( auto_dispose(watcher), ac );
  }
  return watcher;
}
