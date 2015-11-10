#ifndef __server__
#define __server__

#include <sys/types.h>
#include "usr.h"
#include "list.h"
#include "session.h"
#include "message.h"

#define MAX_SESSIONS 20
#define BACKLOG_CONNECTIONS 4

char HELP_STR[MAX_MESSAGE_SIZE] = "\n"
" \\quit: leaves the chat \n"
" \\create <room name> = creates a room \n"
" \\join <room name> = joins a room \n"
" \\leave <room name> = leaves a room \n"
" \\send <message> = broadcasts a message to the room \n"
" \\whisper <who> <message> = sends a message to user 'who' \n"
" \\ls = list all available rooms \n"
" \\nick = changes/define a new nickname \n"
" \\help = shows this \n";

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

// POST-REFACTORING



int SERVER_new_user(SESSION *session, char *buffer);

int SERVER_create_room(SESSION *session, char *buffer);

int SERVER_join_room(SESSION *session, char *buffer);

int SERVER_leave_room(SESSION *session, char *buffer);

int SERVER_room_broadcast(SESSION *session, char *buffer);

int SERVER_send_whisper(SESSION *session, char *buffer);

int SERVER_help(SESSION *session, char *buffer);

int SERVER_list(SESSION *session, char *buffer);

int SERVER_session_disconnect(SESSION *session);

int SERVER_send_message(int socket, char *buffer);

int SERVER_invalid_command(char *buffer);

// PRE-REFACTORING

void SERVER_show_rooms(int socket);

int SERVER_change_user_name(char *user_name, char *new_name);

void SERVER_process_user_cmd (int socket, char *userName);

int SERVER_checkRoomExists (char *roomName);

USER *SERVER_get_user_by_name(char *userName);

ROOM *SERVER_get_room_by_name(char *roomName);

SESSION *SERVER_get_session_by_user(char *userName);


#endif
