#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <getopt.h>
#include "lists.h"
#include "cmd.h"


struct _node_ {
    int value;
    struct list_head list;
};


void show_list(struct list_head *phead)
{
	struct list_head *lp;
	struct _node_ *entry;
	
	printf("show node in list: ");
	list_for_each(lp, phead) {
		entry = list_entry(lp, struct _node_, list);
		printf("[%d]==>",entry->value);
	}
	printf("NULL\n");
}

//
//  function: cmd_list
//      demo how to use list macro and functions
//  parameters
//      argc: 	0
//      argv:   none
//
void cmd_list(int argc, char* argv[])
{
	int i;
	struct _node_ *entry, *next;
	struct list_head *lp;
	
	LIST_HEAD(head);
	
	printf("initial a list\n add node ");
	for (i=0;i<5;i++)
	{	
		entry=(struct _node_ *)malloc(sizeof(struct _node_));
		if (entry)
		{	
			entry->value = i+1;
			list_add_tail( &entry->list, &head);
			printf("%d, ",entry->value);
		}
	}
	printf("\n");
	
	show_list(&head);
	
	printf("remove a node from head\n");
	entry = list_first_entry(&head, struct _node_, list);
	list_del(&entry->list);
	
	show_list(&head);
	
	printf("add a node to tail\n");
	list_add_tail(&entry->list, &head);
	
	show_list(&head);
	
	printf("remove node 3\n");
	list_for_each(lp, &head) 
	{
		entry = list_entry(lp, struct _node_, list);
		if (entry->value == 3)
		{
			next = list_next_entry(entry,list);
			list_del(&entry->list);
			free(entry);
			lp = &next->list;
		}
	}
	show_list(&head);
	
	printf("remove node all\n");
	while (!list_empty(&head))
	{
		entry = list_last_entry(&head, struct _node_, list);
		list_del(&entry->list);
		free(entry);
		show_list(&head);
	}	
	printf("all entry free\n");	
}
