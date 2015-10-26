#ifndef __usr__
#define __usr__

size_t USER_STRUCT_SIZE = ((3*sizeof(int)) + (16*sizeof(char))); 

/* USER STRUCTURE */
typedef struct user {
        int id;
        char name[16];
        int password;
        bool online;
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
