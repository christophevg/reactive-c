#ifndef __UNKNOWN_H
#define __UNKNOWN_H

#include <stdbool.h>

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

void __let(unknown_t, unknown_t);
void __let_int   (unknown_t, int);
void __let_double(unknown_t, double);
void __let_string(unknown_t, char*);

#define __l2(x, y)    __let(x, y)
#define __l3(x, t, v) __let_##t(x, v)
#define __lx(_1,_2,_3,NAME,...) NAME
#define let(...) __lx(__VA_ARGS__, __l3, __l2, __l1)(__VA_ARGS__)

int *__val_int(unknown_t);
double *__val_double(unknown_t);
char **__val_string(unknown_t);

#define val(t,x) *(__val_##t(x))

bool is_int(unknown_t);
bool is_double(unknown_t);
bool is_string(unknown_t);

#define is(t,x) is_##t(x)

#endif
