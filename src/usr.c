#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/usr.h"
#include "../include/list.h"
#include "../include/room.h"
#include <pthread.h>

pthread_mutex_t usr_mutex = PTHREAD_MUTEX_INITIALIZER;

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
		new->in_chat = 0;
		return new;
	}	
}

int USER_change_name(USER *user, char *name) {
	if(strlen(name) > MAX_USER_NAME)
		return -1;
	pthread_mutex_lock(&usr_mutex);
	strncpy(user->name, name, MAX_USER_NAME);
	pthread_mutex_unlock(&usr_mutex);
	return 0;
}

//FUNDAMENTAL
int USER_join_room(USER *user, ROOM *room) {
	pthread_mutex_lock(&usr_mutex);
	user->in_chat = 1;
	user->room = room;
	pthread_mutex_unlock(&usr_mutex);
	return -1;
}

//FUNDAMENTAL
int USER_leave_room(USER *user) {
	pthread_mutex_lock(&usr_mutex);
	user->in_chat = 0;
	user->room = NULL;
	pthread_mutex_unlock(&usr_mutex);
	return -1;
}

int USER_send_message_to_room(USER *user, MESSAGE *msg, ROOM *room) {
	return -1;
}

int USER_send_private_message(USER *from, USER *to, MESSAGE *message) {
	return -1;
}
