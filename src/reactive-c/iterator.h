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
typedef struct type##_iterator {                                      \
  type##s_t   list;                                                   \
  type##_li_t current;                                                \
} *type##_iterator_t;                                                 \
                                                                      \
type##_iterator_t iterate_##type##s(type##s_t list) {                 \
  type##_iterator_t iter = malloc(sizeof(struct type##_iterator));    \
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

#define foreach(type, list, code) \
{\
type##_iterator_t iter=iterate_##type##s(list); \
for(; more_##type##s(iter); next_##type(iter)) code; \
free(iter);\
}
