#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "iterator.h"

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

  item_li_t li1 = malloc(sizeof(struct item_li));
  li1->ob   = i1;

  item_li_t li2 = malloc(sizeof(struct item_li));
  li2->ob   = i2;
  li1->next = li2;

  item_li_t li3 = malloc(sizeof(struct item_li));
  li3->ob   = i3;
  li3->next = NULL;
  li2->next = li3;

  items_t items = malloc(sizeof(struct items));
  items->first = li1;

  foreach(item, items, {
    printf("%d\n", iter->current->ob->value);
  });

  free(i3);  free(i2);  free(i1);
  free(li3); free(li2); free(li1);
  free(items);

  exit(EXIT_SUCCESS);
}
