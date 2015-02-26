#include <stdlib.h>
#include <glib.h>

#include "reactive-c/api.h"

typedef struct {
  observables_t list;
} observables_f;

void setup(observables_f *ob, gconstpointer _) {
  ob->list = observables_new();
}

void teardown(observables_f *ob, gconstpointer _) {
  free(ob->list);
}

void test(observables_f *fix, gconstpointer _) {
  
}

int main(int argc, char **argv) {
  g_test_init(&argc, &argv, NULL);
  g_test_add("/set 1/test 1", observables_f, NULL, setup, test, teardown );
  exit(EXIT_SUCCESS);
}
