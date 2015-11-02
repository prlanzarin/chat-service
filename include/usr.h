#ifndef __usr__
#define __usr__

#include "room.h"
#include "message.h"

#define USER_STRUCT_SIZE ((3*sizeof(int)) + (16*sizeof(char)))
#define MAX_USER_INPUT 21
#define MAX_USER_NAME 16
#define	MAX_USER_COMMAND 10
#define MAX_USER_COMMAND_ARG 20

/* USER STRUCTURE */
typedef struct user {
        int id;
        char name[MAX_USER_NAME];
        int password;
        int online;
} USER;

/* USER subroutines */

USER *USER_create(char *name, int password);

int USER_login(char *name, int password);

int USER_logoff(USER *user);

//FUNDAMENTAL
int USER_join_room(USER *user, ROOM *room);

//FUNDAMENTAL
int USER_leave_room(USER *user, ROOM *room);

//FUNDAMENTAL
int USER_send_message_to_room(USER *user, MESSAGE *msg, ROOM *room);

int USER_send_private_message(USER *from, USER *to, MESSAGE *message);

#endif
