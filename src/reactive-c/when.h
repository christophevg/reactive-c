// when.h

enum {
  NEVER   = 0,
  IS_DONE = 1,
};

typedef int event_t;

#define is(e) is_##e
#define is_done IS_DONE

#define then(a) a

observable_t when(observable_t, event_t, observable_callback_t);
