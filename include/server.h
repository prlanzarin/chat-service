#ifndef __server__
#define __server__

#include <sys/types.h>

#define MAX_SESSIONS 20
#define BACKLOG_CONNECTIONS 4
#define PORT 4000

#include "usr.h"
#include "list.h"
#include "client.h"

int SOCKADDR_IN_SIZE = sizeof(struct sockaddr_in);

typedef struct server {
        int socket_descriptor;
        struct sockaddr_in server_socket;
        USER *server_admin;
        LIST *rooms;
        CLIENT client_sessions[MAX_SESSIONS]; // w/ online users
} SERVER;

int SERVER_new(SERVER *server, short family, unsigned short port, 
		USER *admin);

int SERVER_new_user();

int SERVER_user_login();

int SERVER_user_logoff();

// FUNDAMENTAL
int SERVER_create_room(USER *user, char *room_name);

// FUNDAMENTAL
int SERVER_delete_room(USER *user, int room_id);

#endif
