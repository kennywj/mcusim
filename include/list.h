
#ifndef _list_
#define _list_
#include <stddef.h>

struct list_head {
 struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
               (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr,type,member)     \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member) \
        list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_last_entry(ptr, type, member) \
        list_entry((ptr)->prev, type, member)

int list_empty(const struct list_head *head);
void list_add(struct list_head *new_lst, struct list_head *head);
void list_add_tail(struct list_head *new_lst, struct list_head *head);
void list_del(struct list_head * entry);

#endif //_list_