#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/list.h"

void LIST_push(LIST **head, void *new_node, size_t node_size) {
	LIST *ptr = (LIST *)malloc(sizeof(LIST));

	//ptr->node = malloc(node_size);	
	ptr->node = new_node;
	ptr->next = *head;
	//memcpy(ptr->node, new_node, node_size);
	*head =	ptr;
	return;
}

//REMINDER: Free the allocated node too!
void LIST_remove(LIST **head, LIST *node) {
	return;
}

void LIST_add_user(LIST **head, USER *new_user) {
	LIST_push(head, (void *) new_user, USER_STRUCT_SIZE);
}

void LIST_add_room(LIST **head, ROOM *new_room) {
	LIST_push(head, (void *) new_room, ROOM_STRUCT_SIZE);

}

void LIST_add_session(LIST **head, SESSION *new_session) {
	LIST_push(head, (void *) new_session, SESSION_STRUCT_SIZE);
}
