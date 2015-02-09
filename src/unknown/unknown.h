#ifndef __UNKNOWN_H
#define __UNKNOWN_H

typedef struct Unknown   *unknown_t;
typedef struct internals *internals_t;

// expose public part with accessors to retrieve value
struct Unknown {
  char*  (*as_string) (unknown_t);
  int    (*as_int)    (unknown_t);
  double (*as_double) (unknown_t);
  internals_t private;
} Unknown;

// constructors
unknown_t new_int    (int value);
unknown_t new_double (double value);
unknown_t new_string (char *value);

// syntactic sugar
#define new(t,v) new_##t(v)
#define as(t,v) v->as_##t(v)

void let(unknown_t, unknown_t);

#endif
