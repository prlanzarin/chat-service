#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/room.h"
#include "../include/list.h"
#include "../include/message.h"
#include "../include/usr.h"
#include <pthread.h>
pthread_mutex_t roomMutex = PTHREAD_MUTEX_INITIALIZER;

// TODO FUNDAMENTAL
ROOM *ROOM_create(char *name, USER *creator) {
	if(strlen(name) > MAX_ROOM_NAME)
		return NULL;

	ROOM *new =  calloc(sizeof(ROOM), 1);
	if (!new)
	{
		fprintf(stderr, "ERROR: room creation failed.\n");
		return NULL;
	}
	else
	{
		pthread_mutex_lock(&roomMutex);		
		strncpy(new->name, name, sizeof(new->name));
		new->room_admin = creator;
		new->online_users = NULL;
		pthread_mutex_unlock(&roomMutex);
		return new;
	}	
}

int ROOM_close(int room_id, LIST *rooms) {
	LIST *checkList = rooms;
	ROOM *checkRoom;
	while (checkList)
	{
		checkRoom = (ROOM *) (checkList->node);
		if (checkRoom->id == room_id)
		{
			pthread_mutex_lock(&roomMutex);
			LIST_remove(rooms, checkList);
			pthread_mutex_unlock(&roomMutex);
			return 1;
		}
	}
	fprintf(stderr, "ERROR: room with id %d does not exist.\n", room_id);
	return -1;
}

int ROOM_kick_user(ROOM *room, USER *user) {
	pthread_mutex_lock(&roomMutex);
	LIST_remove_user((room->online_users), (LIST *) user);
	USER_leave_room(user);
	pthread_mutex_unlock(&roomMutex);
	return 1;
}

void ROOM_add_user(ROOM *room, USER *user) {
	pthread_mutex_lock(&roomMutex);
	room->online_users = LIST_push(room->online_users, user);
	USER_join_room(user, room);
	pthread_mutex_unlock(&roomMutex);
	return;
}
