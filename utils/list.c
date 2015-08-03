#include "list.h"
#include <stdlib.h>

daku_list *daku_list_create(void *val)
{
    daku_list *ret = (daku_list *)malloc(sizeof(daku_list));
    ret->head = ret->tail = (daku_list_block *)malloc(sizeof(daku_list_block));
    ret->head->data = val;
    ret->head->next = NULL;
    return ret;
}

void daku_list_push_front(daku_list *list, void *val)
{
    daku_list_block *newblk = (daku_list_block *)malloc(sizeof(daku_list_block));
    newblk->data = val;
    newblk->next = list->head;
    list->head = newblk;
}

void daku_list_push_back(daku_list *list, void *val)
{
    daku_list_block *newblk = (daku_list_block *)malloc(sizeof(daku_list_block));
    newblk->data = val;
    newblk->next = NULL;
    list->tail->next = newblk;
    list->tail = newblk;
}

void *daku_list_at(daku_list *list, int idx)
{
    daku_list_block *blk = list->head;
    while (idx--)
        if ((blk = blk->next) == NULL) return NULL;
    return blk->data;
}

void daku_list_insert(daku_list *list, daku_list_block *after, void *val)
{
    if (after == NULL) daku_list_push_front(list, val);
    else {
        daku_list_block *newblk = (daku_list_block *)malloc(sizeof(daku_list_block));
        newblk->data = val;
        newblk->next = after->next;
        if (after->next == NULL) list->tail = newblk;
        after->next = newblk;
    }
}
