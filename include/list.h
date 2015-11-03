#ifndef __list__
#define __list__

#include "usr.h"
#include "room.h"
#include "session.h"

/* LINKED LIST OF ANY TYPE (void ptr). PLEASE BE CAUTIOUS! */
typedef struct list{
        void *node;
        struct list *next;
} LIST;

LIST *LIST_create(void *new_node);

LIST *LIST_push(LIST *head, void *new_node); 

int LIST_remove(LIST *head, LIST *node);

#endif
