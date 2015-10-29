#ifndef __server__
#define __server__

#include <sys/types.h>
#include "usr.h"
#include "list.h"
#include "session.h"
#include "../include/message.h"

#define MAX_SESSIONS 20
#define BACKLOG_CONNECTIONS 4

size_t SOCKADDR_IN_SIZE = sizeof(struct sockaddr_in);

typedef struct server {
        int socket_descriptor;
        struct sockaddr_in server_socket;
        USER *server_admin;
        LIST *rooms;
        SESSION sessions[MAX_SESSIONS]; // w/ online users
} SERVER;

extern SERVER *server;

SERVER *SERVER_new(short family, unsigned short port, 
		USER *admin);

int SERVER_new_user();

int SERVER_user_login();

int SERVER_user_logoff();

// FUNDAMENTAL
int SERVER_create_room(USER *user, char *room_name);

// FUNDAMENTAL
int SERVER_delete_room(USER *user, int room_id);

#endif
