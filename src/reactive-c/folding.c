#include "observable.h"
#include "observables.h"
#include "internals.h"

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
