#include "observable.h"
#include "internals.h"

// filter support
void _copy_value(observation_t ob) {
  // redirect value to the value of the emitting merged observable
  observable_value_copy(ob->source, ob->self);

  // cached args are out of date, because we're modifying the pointer itself
  // force refresh cache on all observers of merged_ob
  _update_observers_args(ob->self);
}

observable_t __filter(int size, observable_t observable, validator_t validator) {
  observable_t filter = observe(just(observable), _copy_value, size);
  filter->validate = validator;
  return filter;
}
