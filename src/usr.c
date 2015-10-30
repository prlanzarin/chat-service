#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/usr.h"
#include "../include/list.h"
#include "../include/room.h"
#include <pthread.h>

USER *USER_create(char *name, int password) {
	USER *new =  calloc(sizeof(USER), 1);
	if (!new)
	{
		fprintf(stderr, "ERROR: user creation failed.\n");
		return NULL;
	}
	else
	{
		//TODO: user id
		strncpy(new->name, name, sizeof(new->name));
		new->password = password;
		new->online = 0;
		return new;
	}	
}

int USER_login(char *name, int password) {
	return -1;
}

int USER_logoff(USER *user) {
	return -1;
}

//FUNDAMENTAL
int USER_join_room(USER *user, ROOM *room) {
	pthread_mutex_lock(&roomMutex);
	LIST_add_user (&(room->online_users), user);
	pthread_mutex_unlock(&roomMutex);
	return -1;
}

//FUNDAMENTAL
int USER_leave_room(USER *user, ROOM *room) {
	pthread_mutex_lock(&roomMutex);
	LIST_remove (&(room->online_users), (LIST *) user);
	pthread_mutex_unlock(&roomMutex);
	return -1;
}

int USER_send_message_to_room(USER *user, MESSAGE *msg, ROOM *room) {
	return -1;
}

int USER_send_private_message(USER *from, USER *to, MESSAGE *message) {
	return -1;
}
