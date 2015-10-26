#ifndef __server__
#define __server__

#include <sys/types.h>
#include "../include/list.h"

#define MAX_SESSIONS 20
#define BACKLOG_CONNECTIONS 4
#define PORT 4000

int SOCKADDR_IN_SIZE = sizeof(struct sockaddr_in);

typedef struct server {
        int socket_descriptor;
        struct sockaddr_in server_socket;
        USER *server_admin;
        LIST *users;
        LIST *rooms;
        CLIENT client_sessions[MAX_SESSIONS];
} SERVER;

int SERVER_init(SERVER *server);

int SERVER_new_user();

int SERVER_user_login();

int SERVER_user_logoff();

// FUNDAMENTAL
int SERVER_create_room(USER *user, char *room_name);

// FUNDAMENTAL
int SERVER_delete_room(USER *user, int room_id);

#endif
