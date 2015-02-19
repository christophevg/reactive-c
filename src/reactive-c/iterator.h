// macro for iterating a linked list adhering to a few conventions:
// - single linked list of <type>
// - list can be NULL
// - list contains pointer to ->first item
// - next item is accessed through ->next
// - NULL terminated
// the implementation is protected against removal of the iterated item

#define foreach(type, item, list) \
    for(type item = (list? list->first: NULL), \
            _next = (item? item->next : NULL); \
        item != NULL; \
        item = _next, _next = (item? item->next : NULL) \
    )
