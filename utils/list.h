#ifndef __DAKU_LIST_H__
#define __DAKU_LIST_H__

/**
 * @file
 * @ingroup Utilities
 * A simple linked list implementation.
 *
 * Don't seem to need documents...?
 * See the tests for usage, if you need to confirm your guess.
 */
// TODO: Use blocks in linked lists

typedef struct __daku_list_block {
    void *data;
    struct __daku_list_block *next;
} daku_list_block;

typedef struct __daku_list {
    struct __daku_list_block *head, *tail, *itr;
} daku_list;

daku_list *daku_list_create(void *val);
void daku_list_push_front(daku_list *list, void *val);
void daku_list_push_back(daku_list *list, void *val);
void *daku_list_at(daku_list *list, int idx);
void daku_list_insert(daku_list *list, daku_list_block *after, void *val);
#define daku_list_foreach(__list, __itrname)    \
    for ((__list)->itr = (__list)->head,        \
        __itrname = (__list)->itr->data;        \
        (__list)->itr != NULL;                  \
        (__list)->itr = (__list)->itr->next,    \
        __itrname = (__list)->itr ? (__list)->itr->data : NULL)
// XXX: Added before complete C++ support to be compatible with VS. XD
#ifdef __cplusplus
#define daku_list_foreach_t(__list, __type, __itrname) \
    for ((__list)->itr = (__list)->head,            \
        __itrname = reinterpret_cast<__type>((__type)(__list)->itr->data);    \
        (__list)->itr != NULL;                      \
        (__list)->itr = (__list)->itr->next,        \
        __itrname = (__list)->itr ? reinterpret_cast<__type>((__list)->itr->data) : NULL)
#else
#define daku_list_foreach_t(__list, __type, __itrname) \
    for ((__list)->itr = (__list)->head,            \
        __itrname = (__type)(__list)->itr->data;    \
        (__list)->itr != NULL;                      \
        (__list)->itr = (__list)->itr->next,        \
        __itrname = (__list)->itr ? (__type)(__list)->itr->data : NULL)
#endif

#endif
