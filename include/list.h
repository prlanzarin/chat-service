#ifndef __list__
#define __list__

#include "../include/usr.h"
#include "../include/room.h"
#include "../include/client.h"

/* LINKED LIST OF ANY TYPE (void ptr). PLEASE BE CAUTIOUS! */
typedef struct list{
        void *node;
        LIST *next;
} LIST;

void LIST_add_user(LIST **head, USER *new_user);

void LIST_add_room(LIST **head, ROOM *new_room);

void LIST_add_session(LIST **head, CLIENT *new_session);

#endif
