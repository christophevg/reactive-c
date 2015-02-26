#include "observable.h"
#include "internals.h"

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
