#ifndef __DAKU_LIST_H__
#define __DAKU_LIST_H__

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
#define daku_list_foreach(__list, __itrname)    \
    for ((__list)->itr = (__list)->head,        \
        __itrname = (__list)->itr->data;        \
        (__list)->itr->next != NULL;            \
        (__list)->itr = (__list)->itr->next,    \
        __itrname = (__list)->itr->data)

#endif
