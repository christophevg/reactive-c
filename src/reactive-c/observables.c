// observables

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "iterator.h"

// turns a variadic list of observables into an linked list. this is a helper
// function to allow calling observe(each(...) and pass a variable list of
// observables)
observables_t __each(int count, ...) {
  va_list       ap;
  observables_t list = observables_new();
  
  // import observables
  va_start(ap, count);
  for(int i=0; i<count; i++) {
    observables_add(list, va_arg(ap, observable_t));
  }
  va_end(ap);
  
  // return casted version
  return list;
}

// constructor and accessors for an observables list
observables_t observables_new() {
  observables_t list = malloc(sizeof(struct observables));
  list->first = NULL;
  list->last  = NULL;
  return list;
}

void observables_add(observables_t list, observable_t observable) {
  observable_li_t item = malloc(sizeof(struct observable_li));
  item->ob             = observable;
  item->next           = NULL;
  item->prop           = 0;
  if(list->last != NULL) { list->last->next = item; }
  list->last           = item;
  if(list->first == NULL) { list->first = item; }
}

observables_t observables_dup(observables_t originals) {
  observables_t list = observables_new();
  foreach(observable_li_t, original, originals) {
    observables_add(list, original->ob);
  }
  return list;
}

void observables_remove(observables_t list, observable_t observable) {
  if(observables_is_empty(list)) {
    assert(list->last == NULL);
    return; // empty list
  }
  observable_li_t head = list->first;
  if(head->ob == observable) {
    // first in the list, remove the head
    list->first = head->next;
    free(head);
    if(observables_is_empty(list)) { list->last = NULL; } // it was also the only item
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

void observables_clear(observables_t list) {
  if(list == NULL) { return; }
  foreach(observable_li_t, item, list) {
    free(item);
  }
  list->first = NULL;
  list->last = list->first;
}

bool observables_contains(observables_t list, observable_t observable) {
  foreach(observable_li_t, item, list) {
    if(item->ob == observable) { return true; }
  }
  return false;
}

bool observables_is_empty(observables_t list) {
  return list->first == NULL;
}

int observables_count(observables_t list) {
  int count = 0;
  foreach(observable_li_t, _, list) { count++; }
  return count;
}

// generic constructor for observables that observe a set of observables
observable_t _combine(char *label, observables_t observeds, observer_t handler) {
  observable_t combination = start(__observing(label, observeds, handler, 0));
  return combination;
}
