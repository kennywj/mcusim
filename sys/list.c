#include <stddef.h>
#include "list.h"

static  void __list_add(struct list_head *new_lst, struct list_head *prev, struct list_head *next)
{
        next->prev = new_lst;
        new_lst->next = next;
        new_lst->prev = prev;
        prev->next = new_lst;
}

static  void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

int list_empty(const struct list_head *head)
{
    return head->next == head;
}

void list_add(struct list_head *new_lst, struct list_head *head)
{
	__list_add(new_lst, head, head->next);
}

void list_add_tail(struct list_head *new_lst, struct list_head *head)
{
    __list_add(new_lst, head->prev, head);
}

void list_del(struct list_head * entry)
{
	__list_del(entry->prev,entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}
