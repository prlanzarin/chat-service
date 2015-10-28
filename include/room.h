#ifndef __room__
#define __room__

typedef struct user USER; 
typedef struct message MESSAGE; 
typedef struct list LIST; 

#define ROOM_STRUCT_SIZE 0

typedef struct room {
        int id;
        char name[16];
        USER **room_admin;
        LIST *online_users;
} ROOM;

// FUNDAMENTAL
ROOM *ROOM_create(char *name);

// FUNDAMENTAL
int ROOM_close(int room_id);

// FUNDAMENTAL
int ROOM_broadcast_message(ROOM *room, MESSAGE *msg);

int ROOM_kick_user(ROOM *room, USER *user);

int ROOM_add_user(ROOM *room, USER *user);

#endif
