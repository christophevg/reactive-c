// macro for setting up a linked list given a type with iterator
// given <type> this macro generates:
// - <type>_li_t    : list item structure pointer
// - <type>s_t      : list structure pointer
// - <type>_iter_t  : iterator struct pointer
// - new_<type>s()  : constructor for new list
// - add_<type>(<type>s_t, <type>_t)    : adds a <type> to the list
// - remove_<type>(<type>s_t, <type>_t) : removes as <type> from the list
// - clear_type(s)(<type>s_t)           : clears the list

#define construct_iterator(type)                                      \
                                                                      \
typedef struct {                                                      \
  type##s_t   list;                                                   \
  type##_li_t current;                                                \
} *type##_iterator_t;                                                 \
                                                                      \
type##_iterator_t iterate_##type##s(type##s_t list) {                 \
  type##_iterator_t iter = malloc(sizeof(type##_iterator_t));         \
  iter->list = list;                                                  \
  iter->current = list->first;                                        \
  return iter;                                                        \
}                                                                     \
                                                                      \
bool more_##type##s(type##_iterator_t iter) {                         \
  return iter->current != NULL;                                       \
}                                                                     \
                                                                      \
void next_##type(type##_iterator_t iter) {                            \
  iter->current = iter->current->next;                                \
}

#define foreach(type, list) \
for(type##_iterator_t iter=iterate_##type##s(list); more_##type##s(iter); next_##type(iter))

///////////////////

#ifdef _RUN_EXAMAPLE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct item {
  int value;
} *item_t;

typedef struct item_li {
  item_t ob;
  struct item_li *next;
} *item_li_t;

typedef struct items {
  item_li_t first;
} *items_t;

construct_iterator(item);

int main(void) {
  item_t i1 = malloc(sizeof(item_t));
  i1->value = 1;
  item_t i2 = malloc(sizeof(item_t));
  i2->value = 2;
  item_t i3 = malloc(sizeof(item_t));
  i3->value = 3;

  item_li_t li1 = malloc(sizeof(item_li_t));
  li1->ob   = i1;

  item_li_t li2 = malloc(sizeof(item_li_t));
  li2->ob   = i2;
  li1->next = li2;

  item_li_t li3 = malloc(sizeof(item_li_t));
  li3->ob   = i3;
  li2->next = li3;
  
  items_t items = malloc(sizeof(items_t));
  items->first = li1;
  
  foreach(item, items) {
    printf("%d\n", iter->current->ob->value);
  }
  
  exit(EXIT_SUCCESS);
}

#endif
