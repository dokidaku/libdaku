#include "list.h"
#include <stdlib.h>

daku_list *daku_list_create(void *val)
{
    daku_list *ret = (daku_list *)malloc(sizeof(daku_list));
    ret->head = ret->tail = (daku_list_block *)malloc(sizeof(daku_list_block));
    ret->head->data = val;
    ret->head->next = NULL;
    ret->length = 1;
    return ret;
}

void daku_list_push_front(daku_list *list, void *val)
{
    daku_list_block *newblk = (daku_list_block *)malloc(sizeof(daku_list_block));
    newblk->data = val;
    newblk->next = list->head;
    list->head = newblk;
    ++list->length;
}

void daku_list_push_back(daku_list *list, void *val)
{
    daku_list_block *newblk = (daku_list_block *)malloc(sizeof(daku_list_block));
    newblk->data = val;
    newblk->next = NULL;
    list->tail->next = newblk;
    list->tail = newblk;
    ++list->length;
}

daku_list_block *daku_list_itr_at(daku_list *list, int idx)
{
    daku_list_block *blk = list->head;
    while (idx--)
        if ((blk = blk->next) == NULL) return NULL;
    return blk;
}

void *daku_list_at(daku_list *list, int idx)
{
    daku_list_block *blk = daku_list_itr_at(list, idx);
    return blk ? blk->data : NULL;
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
        ++list->length;
    }
}

void daku_list_remove(daku_list *list, daku_list_block *after)
{
    daku_list_block *rmblk;
    if (after == NULL) rmblk = list->head;
    else if ((rmblk = after->next) == NULL) return;
    if (rmblk == list->tail) {
        if (rmblk == list->head) return; // Not permitted right now :(
        list->tail = after;
        after->next = NULL;
    } else if (rmblk == list->head) {
        list->head = rmblk->next;
    } else {
        after->next = rmblk->next;
    }
    free(rmblk);
    --list->length;
}
