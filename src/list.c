#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/list.h"
#include "../include/pthread.h"

pthread_mutex_t lst_mutex = PTHREAD_MUTEX_INITIALIZER;

LIST *LIST_create(void *new_node) {
	LIST *node;
	node = (LIST *) malloc(sizeof(LIST));
	node->node = new_node;
	node->next = NULL;
	return node;
}

LIST *LIST_push(LIST *head, void *new_node) {
	pthread_mutex_lock(&lst_mutex);
	LIST *node = LIST_create(new_node);	
	if(head == NULL) {
		head = node;
		head->next = NULL;	
		pthread_mutex_unlock(&lst_mutex);
		return head;
	}
	pthread_mutex_unlock(&lst_mutex);
	node->next = head;
	return node;
}

int LIST_remove(LIST *head, LIST *node)
{
	pthread_mutex_lock(&lst_mutex);
	while(head->next && head->next!=node) 
		head = head->next;
	if(head->next) {
		head->next = node->next;
		free(node);
		pthread_mutex_unlock(&lst_mutex);
		return 0;		
	} 
	else {
		pthread_mutex_unlock(&lst_mutex);
		return -1;
	}
}

int LIST_remove_user(LIST *head, LIST *node)
{
	pthread_mutex_lock(&lst_mutex);
	USER *u2, *target;
	u2 = (USER *) head->next;
	target = (USER *)node;
	while(head->next && strcmp(u2->name, target->name)) {
		head = head->next;
		u2 = (USER *) head->next;
	}
	if(head->next) {
		head->next = node->next;
		pthread_mutex_unlock(&lst_mutex);
		return 0;		
	} 
	else {
		pthread_mutex_unlock(&lst_mutex);
		return -1;
	}
}
