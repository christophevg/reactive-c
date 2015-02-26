#include <stdlib.h>
#include <glib.h>

#include "reactive-c/api.h"

typedef struct {
  observables_t list;
  int _a, _b, _c;
  observable_t a, b, c;
} observables_f;

void setup_empty_list(observables_f *ob, gconstpointer _) {
  ob->list = observables_new();
  ob->a = NULL;
  ob->b = NULL;
  ob->c = NULL;
}

void teardown(observables_f *ob, gconstpointer _) {
  free(ob->list);
  if(ob->a) { free(ob->a); }
  if(ob->b) { free(ob->b); }
  if(ob->c) { free(ob->c); }
}

void test_add(observables_f *ob, gconstpointer _) {
  g_assert(observables_is_empty(ob->list));
  int _d = 123;
  observable_t dummy = observe(int, _d);
  observables_add(ob->list, dummy);
  g_assert(! observables_is_empty(ob->list));
  g_assert(observables_count(ob->list) == 1);
}

void setup_list_with_three_observable(observables_f *ob, gconstpointer _) {
  ob->list = observables_new();

  ob->_a = 1;
  ob->_b = 2;
  ob->_c = 3;

  ob->a = observe(int, ob->_a);
  ob->b = observe(int, ob->_b);
  ob->c = observe(int, ob->_c);
  
  observables_add(ob->list, ob->a);
  observables_add(ob->list, ob->b);
  observables_add(ob->list, ob->c);
}

void test_remove(observables_f *ob, gconstpointer _) {
  observables_remove(ob->list, ob->b);
  g_assert(!observables_is_empty(ob->list));
  g_assert(observables_count(ob->list) == 2);

  observables_remove(ob->list, ob->c);
  g_assert(!observables_is_empty(ob->list));
  g_assert(observables_count(ob->list) == 1);

  observables_remove(ob->list, ob->a);
  g_assert(observables_is_empty(ob->list));
  g_assert(observables_count(ob->list) == 0);
}

void test_clear(observables_f *ob, gconstpointer _) {
  observables_clear(ob->list);
  g_assert(observables_is_empty(ob->list));
}

void test_contains(observables_f *ob, gconstpointer _) {
  g_assert(observables_contains(ob->list, ob->a));
  g_assert(observables_contains(ob->list, ob->b));
  g_assert(observables_contains(ob->list, ob->c));
  int _u = 123;
  observable_t unknown = observe(int, _u);
  g_assert( ! observables_contains(ob->list, unknown));
}

int main(int argc, char **argv) {
  g_test_init(&argc, &argv, NULL);
  
  g_test_add("/observables/add", observables_f, NULL,
             setup_empty_list, test_add, teardown );
  g_test_add("/observables/remove", observables_f, NULL,
             setup_list_with_three_observable, test_remove, teardown );
  g_test_add("/observables/clear", observables_f, NULL,
             setup_list_with_three_observable, test_clear, teardown );
  g_test_add("/observables/contains", observables_f, NULL,
             setup_list_with_three_observable, test_contains, teardown );

  return g_test_run();
}
