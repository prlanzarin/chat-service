#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/room.h"
#include "../include/list.h"
#include "../include/message.h"
#include <pthread.h>
pthread_mutex_t roomMutex = PTHREAD_MUTEX_INITIALIZER;

// TODO FUNDAMENTAL
ROOM *ROOM_create(char *name, USER *creator) {

	ROOM *new =  calloc(sizeof(ROOM), 1);
	if (!new)
	{
		fprintf(stderr, "ERROR: room creation failed.\n");
		return NULL;
	}
	else
	{
		//TODO: room id
		strncpy(new->name, name, sizeof(new->name));
		new->room_admin = &creator;
		pthread_mutex_lock(&roomMutex);		
		pthread_mutex_unlock(&roomMutex);
		return new;
	}	
}

// TODO FUNDAMENTAL
int ROOM_close(int room_id, LIST *rooms) {
	LIST *checkList = rooms;
	ROOM *checkRoom;
	while (checkList)
	{
		checkRoom = (ROOM *) (checkList->node);
		if (checkRoom->id == room_id)
		{
			pthread_mutex_lock(&roomMutex);
			LIST_remove (&rooms, checkList);
			pthread_mutex_unlock(&roomMutex);
			return 1;
		}
	}
	fprintf(stderr, "ERROR: room with id %d does not exist.\n", room_id);
	return -1;
}

// TODO FUNDAMENTAL
int ROOM_broadcast_message(ROOM *room, MESSAGE *msg) {
	return -1;
}

// TODO 
int ROOM_kick_user(ROOM *room, USER *user) {
	pthread_mutex_lock(&roomMutex);
	LIST_remove (&(room->online_users), (LIST *) user);
	pthread_mutex_unlock(&roomMutex);
	return -1;
}

// TODO 
int ROOM_add_user(ROOM *room, USER *user) {
	pthread_mutex_lock(&roomMutex);
	LIST_add_user (&(room->online_users), user);
	pthread_mutex_unlock(&roomMutex);
	return -1;
}
