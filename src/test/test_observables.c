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
  observable_t dummy = observe(_d);
  observables_add(ob->list, dummy);
  g_assert(! observables_is_empty(ob->list));
  g_assert(observables_count(ob->list) == 1);
}

void setup_list_with_three_observable(observables_f *ob, gconstpointer _) {
  ob->list = observables_new();

  ob->_a = 1;
  ob->_b = 2;
  ob->_c = 3;

  ob->a = observe(ob->_a);
  ob->b = observe(ob->_b);
  ob->c = observe(ob->_c);
  
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
  observable_t unknown = observe(_u);
  g_assert( ! observables_contains(ob->list, unknown));
}

void test_dup(observables_f *ob, gconstpointer _) {
  observables_t dup = observables_dup(ob->list);
  g_assert(observables_contains(dup, ob->a));
  g_assert(observables_contains(dup, ob->b));
  g_assert(observables_contains(dup, ob->c));
}

void test_insert_by_level(observables_f *ob, gconstpointer _) {
  g_assert(observables_is_empty(ob->list));
  int value = 0;
  observable_t o1 = force_level(observe(value), 1);
  observable_t o2 = force_level(observe(value), 2);
  observable_t o3 = force_level(observe(value), 3);
  observables_insert_unique_by_level(ob->list, o2);
  observables_insert_unique_by_level(ob->list, o3);
  observables_insert_unique_by_level(ob->list, o1);

  g_assert(observables_count(ob->list) == 3);

  observable_t o;

  o = observables_first(ob->list);
  g_assert(o == o1);
  observables_remove(ob->list, o);

  o = observables_first(ob->list);
  g_assert(o == o2);
  observables_remove(ob->list, o);

  o = observables_first(ob->list);
  g_assert(o == o3);
  observables_remove(ob->list, o);
}

void test_insert_unique(observables_f *ob, gconstpointer _) {
  g_assert(observables_is_empty(ob->list));
  int value = 0;
  observable_t o1 = force_level(observe(value), 1);
  observable_t o2 = force_level(observe(value), 2);
  observable_t o3 = force_level(observe(value), 3);

  // this simulates the glitch scenario
  // 0 adds 1 and 2
  observables_insert_unique_by_level(ob->list, o1);
  observables_insert_unique_by_level(ob->list, o2);

  g_assert(observables_count(ob->list) == 2);

  // 1 gets updated
  observables_remove(ob->list, o1);
  observables_insert_unique_by_level(ob->list, o3);

  g_assert(observables_count(ob->list) == 2);

  // 2 gets updated  
  observables_remove(ob->list, o2);
  observables_insert_unique_by_level(ob->list, o3);

  // only 3 should still be in the list
  g_assert(observables_count(ob->list) == 1);
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
  g_test_add("/observables/dup", observables_f, NULL,
             setup_list_with_three_observable, test_dup, teardown );
  g_test_add("/observables/insert_by_level", observables_f, NULL,
             setup_empty_list, test_insert_by_level, teardown );
  g_test_add("/observables/insert_unique", observables_f, NULL,
             setup_empty_list, test_insert_unique, teardown );
  return g_test_run();
}
