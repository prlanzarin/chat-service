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
	LIST *users; //includes offline and online users
        SESSION sessions[MAX_SESSIONS]; // w/ online users
} SERVER;

extern SERVER *server;

SERVER *SERVER_new(short family, unsigned short port, 
		USER *admin);

int SERVER_new_user(int socket_desc, USER *newUser);

int SERVER_user_login();

int SERVER_user_logoff();

// FUNDAMENTAL
int SERVER_create_room(USER *user, char *room_name);

// FUNDAMENTAL
int SERVER_delete_room(USER *user, int room_id);

void SERVER_show_rooms(int socket);

int SERVER_change_user_name(char *user_name, char *new_name);

void SERVER_process_user_cmd (int socket, char *userName);

int SERVER_checkRoomExists (char *roomName);

USER *SERVER_get_user_by_name(char *userName);

ROOM *SERVER_get_room_by_name(char *roomName);

SESSION *SERVER_get_session_by_user(char *userName);

void SERVER_send_message(char *userName, char *message);

#endif
