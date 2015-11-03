#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/list.h"

LIST *LIST_create(void *new_node) {
	LIST *node;
	node = (LIST *) malloc(sizeof(LIST));
	node->node = new_node;
	node->next = NULL;
	return node;
}

LIST *LIST_push(LIST *head, void *new_node) {
	LIST *node = LIST_create(new_node);	
	node->next = head;
	return node;
}

int LIST_remove(LIST *head, LIST *node)
{
	while(head->next && head->next!=node) 
		head = head->next;
	if(head->next) {
		head->next = node->next;
		free(node);
		return 0;		
	} 
	else 
		return -1;
}
