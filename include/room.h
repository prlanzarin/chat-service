#ifndef __room__
#define __room__

typedef struct user USER; 
typedef struct message MESSAGE; 
typedef struct list LIST; 

#define ROOM_STRUCT_SIZE (sizeof(int) + (sizeof(char) * MAX_ROOM_NAME) + sizeof(USER *) + sizeof(LIST *))
#define MAX_ROOM_NAME 16

typedef struct room {
        int id;
        char name[MAX_ROOM_NAME];
        USER *room_admin;
        LIST *online_users;
} ROOM;

extern pthread_mutex_t roomMutex;

// FUNDAMENTAL
ROOM *ROOM_create(char *name, USER *creator); 

// FUNDAMENTAL
int ROOM_close(int room_id, LIST *rooms);

// FUNDAMENTAL
int ROOM_broadcast_message(ROOM *room, MESSAGE *msg);

int ROOM_kick_user(ROOM *room, USER *user);

void ROOM_add_user(ROOM *room, USER *user);

#endif
