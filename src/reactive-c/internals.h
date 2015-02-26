#ifndef __OBSERVABLE_INTERNALS_H
#define __OBSERVABLE_INTERNALS_H

// detailed structure of an observable
// WARNING: this header should _ONLY_ be included by Reactive C's own modules

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
#define _is_value(o)          (o->prop &   VALUE)
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
#define _is_script(o)         (o->next && o->parent == NULL)

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

// observation internal extension
// alternative:
// 2. a pointer to a participants structure, containing pointers to the emitter
//    and receiver (internal use only)
typedef struct participants {
  observable_t source;
  observable_t target;
} *participants_t;

// a generic type for handlers dealing with an observable
typedef void (*observable_handler_t)(observable_t);

// observable

observable_t _new(char*);
void         _free(observable_t);
observable_t _update_level(observable_t);
void         _update_observers_args(observable_t);
observable_t _update_args(observable_t);
observable_t _clear_observers(observable_t);
observable_t _clear_observeds(observable_t);

void _on_update(observable_handler_t);

// observables

// linked list item (LI) for observables. "next" can't be added to observable
// because each observable can be grouped with other observers or observed
// observables.
typedef struct observable_li {
  observable_t         ob;
  int                  prop;
  struct observable_li *next;
} *observable_li_t;

// the list is also packed in a struct, containing the root/first list item and
// a reference to the last, for faster additions.
struct observables {
  observable_li_t first;
  observable_li_t last;
} observables;

observable_t _combine(char*, observables_t, observer_t);

#endif
