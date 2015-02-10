#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "unknown.h"

typedef struct internals {
  union {
    int    i;
    double d;
    char  *s;
  } value;
  enum { INTEGER, DOUBLE, STRING } type;
  char *buffer; // used for conversion to string, allows for better freeing
} internals;

// int ->
char *i2s(unknown_t data) {
  if(data->private->buffer) { free(data->private->buffer); }
  data->private->buffer = malloc(sizeof(char)*16);
  assert(data->private->buffer != NULL);
  snprintf(data->private->buffer, 15,"%d", data->private->value.i);
  return data->private->buffer;
}
int i2i(unknown_t data) { return data->private->value.i; }
double i2d(unknown_t data) { return (double)data->private->value.i; }

// double ->
char *d2s(unknown_t data) {
  if(data->private->buffer) { free(data->private->buffer); }
  data->private->buffer = malloc(sizeof(char)*16);
  assert(data->private->buffer != NULL);
  snprintf(data->private->buffer,15,"%f", data->private->value.d);
  return data->private->buffer;
}
int d2i(unknown_t data) { return (int)(data->private->value.d + 0.5); }
double d2d(unknown_t data) { return data->private->value.d; }

// string ->
char *s2s(unknown_t data) { return data->private->value.s; }
int s2i(unknown_t data) { return atoi(data->private->value.s); }
double s2d(unknown_t data) { return atof(data->private->value.s); }

unknown_t new_int(int value) {
  unknown_t v = malloc(sizeof(Unknown));
  assert(v != NULL);
  v->as_string = i2s;
  v->as_int    = i2i;
  v->as_double = i2d;
  v->private = malloc(sizeof(internals));
  v->private->type = INTEGER;
  v->private->value.i = value;
  v->private->buffer  = NULL;
  return v;
}

unknown_t new_double(double value) {
  unknown_t v = malloc(sizeof(Unknown));
  assert(v != NULL);
  v->as_string = d2s;
  v->as_int    = d2i;
  v->as_double = d2d;
  v->private = malloc(sizeof(internals));
  v->private->type = DOUBLE;
  v->private->value.d = value;
  v->private->buffer  = NULL;
  return v;
}

unknown_t new_string(char *value) {
  unknown_t v = malloc(sizeof(Unknown));
  assert(v != NULL);
  v->as_string = s2s;
  v->as_int    = s2i;
  v->as_double = s2d;
  v->private = malloc(sizeof(internals));
  v->private->type = STRING;
  v->private->value.s = value;
  v->private->buffer  = NULL;
  return v;
}

void let(unknown_t rval, unknown_t lval) {
  switch(rval->private->type) {
    case INTEGER: rval->private->value.i = as(int,    lval); break;
    case DOUBLE:  rval->private->value.d = as(double, lval); break;
    case STRING:  rval->private->value.s = as(string, lval); break;
  }
}

void __let_int(unknown_t rval, int lval) {
  switch(rval->private->type) {
    case INTEGER: rval->private->value.i =         lval; break;
    case DOUBLE:  rval->private->value.d = (double)lval; break;
    case STRING:
      sprintf(rval->private->value.s, "%d", lval);
      break;
  }
}

void __let_double(unknown_t rval, double lval) {
  switch(rval->private->type) {
    case INTEGER: rval->private->value.i = (int)lval;          break;
    case DOUBLE:  rval->private->value.d =      lval;          break;
    case STRING:  sprintf(rval->private->value.s, "%f", lval); break;
  }
}

void __let_string(unknown_t rval, char *lval) {
  switch(rval->private->type) {
    case INTEGER: rval->private->value.i = atoi(lval); break;
    case DOUBLE:  rval->private->value.d = atof(lval); break;
    case STRING:  rval->private->value.s = lval;       break;
  }
}

int *__val_int(unknown_t val) {
  assert(val->private->type == INTEGER);
  return &val->private->value.i;
}

double *__val_double(unknown_t val) {
  assert(val->private->type == DOUBLE);
  return &val->private->value.d;
}

char **__val_string(unknown_t val) {
  assert(val->private->type == STRING);
  return &val->private->value.s;
}
