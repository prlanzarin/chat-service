#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> 
#include "../include/server.h"
#include "../include/room.h"

void *connection_handler(void *);
int free_session_index(SESSION session_list[]);

SERVER *server;
pthread_mutex_t roomListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sessionAssignMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t socketw_mutex = PTHREAD_MUTEX_INITIALIZER;
int curr_session = -1;

int main(int argc , char *argv[])
{
	SESSION *s_ptr;

	if((server = SERVER_new(AF_INET, PORT, NULL)) == NULL) {
		fprintf(stderr, "ERROR: server creation failed.\n");
		exit(EXIT_FAILURE);
	}

	curr_session = free_session_index(server->sessions);	

	// server stays listening on its socket for new client connections
	listen(server->socket_descriptor, BACKLOG_CONNECTIONS);

	// connection capture
	puts("[SERVER] Waiting for incoming connections...");
	s_ptr = &server->sessions[0];
	while(1) {
		if((s_ptr[curr_session].socket_descriptor =
					accept(server->socket_descriptor, 
						(struct sockaddr *) &s_ptr[curr_session].client_socket, 
						(socklen_t*) &SOCKADDR_IN_SIZE)) < 0)
			fprintf(stderr, "ERROR: connection could not be accepted\n");


		if(curr_session != -1) 
			printf("[SERVER] free session found: %d\n", curr_session);
		else { 
			//send_message to client of no free sessions
			SERVER_send_message(s_ptr[curr_session].socket_descriptor,
					"[SERVER] NO FREE SESSIONS!");
		}
		s_ptr[curr_session].valid = 1;

		if(pthread_create(&s_ptr[curr_session].session_thread, NULL, 
					connection_handler, 
					(void*)&s_ptr[curr_session]) < 0) {
			perror("ERROR: new server thread could not be created");
			exit(EXIT_FAILURE);
		}

		// now join the thread , so that we dont terminate before the thread
		//pthread_join(s_ptr[curr_session].session_thread, NULL);
		puts("[SERVER] handler assigned");
		curr_session = free_session_index(server->sessions);	
	}
	return 0;
}

/*
 * This will handle connection for each client
 */
void *connection_handler(void *in)
{
	//Get the socket descriptor
	SESSION *session = (SESSION *)in;
	int sock = session->socket_descriptor, inc_size;
	int msg_size = 0;
	char buffer[MAX_MESSAGE_SIZE];

	//Send some messages to the client
	if (SERVER_send_message(sock, 
				"[SERVER] Hello! Type \\nick <name> to enter your name:") < 0)
		fprintf(stderr, "ERROR: could not send message to client.");

	while(1) {
		memset(buffer, 0, sizeof(buffer));
		inc_size = read(sock, &msg_size, sizeof(int));
		if(inc_size < 0) {
			fprintf(stderr, "ERROR: could not read from client socket\n");
			exit(EXIT_FAILURE);
		}
		// /quit command
		if(inc_size == 0)
			break;

		printf("SERVER_SIZE_INC: received \"%d\" from %d\n", 
				msg_size, session->socket_descriptor);

		char *ptr = buffer;
		while(msg_size > 0) {
			inc_size = read(sock, ptr, msg_size);
			if(inc_size < 0) {
				fprintf(stderr, "ERROR: could not read from client socket\n");
				exit(EXIT_FAILURE);
			}
			// /quit command
			if(inc_size == 0)
				break;
			msg_size = msg_size - inc_size;
			ptr = ptr + inc_size;
		}

		printf("SERVER_MESSAGE_INC: received \" %s\" from %d\n", 
				buffer, session->socket_descriptor);

		// Main operation decoupling
		if (strstr(buffer, USER_SEND_MESSAGE_TO_ROOM) != NULL)
			SERVER_room_broadcast(session, 
					buffer + strlen(USER_SEND_MESSAGE_TO_ROOM));

		else if (strstr(buffer, USER_LEAVE_ROOM) != NULL)
			SERVER_leave_room(session);

		else if (strstr(buffer, USER_JOIN_ROOM) != NULL)
			SERVER_join_room(session, buffer + strlen(USER_JOIN_ROOM));

		else if (strstr(buffer, QUIT) != NULL) 
			SERVER_session_disconnect(session);

		else if (strstr(buffer, ROOM_CREATION) != NULL)
			SERVER_create_room(session, buffer + strlen(ROOM_CREATION));

		else if (strstr(buffer, USER_NICKNAME) != NULL)
			SERVER_new_user(session, buffer + strlen(USER_NICKNAME));

		else if (strstr(buffer, ROOM_LISTING) != NULL)
			SERVER_list(session); 

		else if (strstr(buffer, HELP) != NULL)
			SERVER_help(session, buffer + strlen(HELP));

		else if (strstr(buffer, USER_SEND_PRIVATE_MESSAGE) != NULL) 
			SERVER_send_whisper(session, 
					buffer + strlen(USER_SEND_PRIVATE_MESSAGE));

		else if (SERVER_invalid_command(buffer))
			SERVER_send_message(sock, "[SERVER] INVALID COMMAND!");

		else 
			SERVER_send_message(sock, "[SERVER] INVALID COMMAND!");
	}

	SERVER_session_disconnect(session);
	pthread_exit(0);
} 

// Changes nickname or sets a new user
int SERVER_new_user(SESSION *session, char *buffer) {
	char *snd_buf = malloc(sizeof(MAX_MESSAGE_SIZE));
	if(USER_change_name(session->user, buffer) < 0)
		SERVER_send_message(session->socket_descriptor, "[SERVER] NAME TOO LARGE");
	else {
		strncpy(snd_buf, "[SERVER] Your new nick is ", MAX_MESSAGE_SIZE);
		strncat(snd_buf, session->user->name, 16);
		SERVER_send_message(session->socket_descriptor, snd_buf);
	}	
	return 0;
}

// Sends a message <buffer> to socket
int SERVER_send_message(int socket, char *buffer) {
	int m_size = 0; 

	pthread_mutex_lock(&socketw_mutex);
	m_size = strlen(buffer) + 1;

	// sending message size to client
	if(write(socket, &m_size, sizeof(int)) < sizeof(int)) {
		fprintf(stderr, "ERROR: could not write on socket %d\n", socket);
		pthread_mutex_unlock(&socketw_mutex);
		return -1;
	}

	printf("MESSAGE_SIZE: sent %d to %d\n", m_size, socket);
	// sending actual message to client
	if(write(socket, buffer, m_size) > 0) {
		printf("MESSAGE: sent \"%s\" to %d\n", buffer, socket);
		pthread_mutex_unlock(&socketw_mutex);
		return 0;
	}
	else {
		fprintf(stderr, "ERROR: could no write on socket %d\n", socket);
		pthread_mutex_unlock(&socketw_mutex);
		return -1;
	}

}

// Creates a room named <buffer> within the server
int SERVER_create_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);
	pthread_mutex_lock(&roomListMutex);
	if (new_room == NULL) // There's no room named <buffer in the server
	{
		ROOM *new_room = ROOM_create(buffer, session->user);	
		if(new_room == NULL) {
			SERVER_send_message(session->socket_descriptor, "[SERVER] NAME TOO LARGE");
			return -1;
		}
		server->rooms = LIST_push(server->rooms, new_room);
		char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
		strncpy(snd_buf, new_room->name, MAX_USER_NAME);
		strncat(snd_buf, " was created!", MAX_MESSAGE_SIZE);
		SERVER_send_message(session->socket_descriptor, snd_buf);
		printf("[SERVER] Room %s was created by %d\n", new_room->name,
				session->socket_descriptor);
		pthread_mutex_unlock(&roomListMutex);
		return 0;
	}
	else	// The room <buffer> already exists within the server
	{
		fprintf(stderr, "ERROR: room name already exists");
		SERVER_send_message(session->socket_descriptor, "[SERVER] ROOM ALREADY EXISTS");
		pthread_mutex_unlock(&roomListMutex);

		return -1;
	}

	pthread_mutex_unlock(&roomListMutex);
	return 0;
}

// Session <session>'s user joins the room named <buffer>
int SERVER_join_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);

	if(new_room == NULL) {
		SERVER_send_message(session->socket_descriptor, 
				"[SERVER] THERE IS NO ROOM WITH THAT NAME");
		return -1;
	}

	if(session->user->in_chat) {
		SERVER_send_message(session->socket_descriptor, 
				"[SERVER] ALREADY ON CHAT");
		return -1;
	}

	char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
	strncpy(snd_buf, session->user->name, MAX_USER_NAME);
	strncat(snd_buf, " joined the room!", MAX_MESSAGE_SIZE);
	SERVER_send_message(session->socket_descriptor, snd_buf);
	ROOM_add_user(new_room, session->user);

	return 0;
}

// <session>'s user leaves the room it's currently in
int SERVER_leave_room(SESSION *session) {
	ROOM *new_room = session->user->room;

	if(new_room == NULL) {
		SERVER_send_message(session->socket_descriptor, 
				"[SERVER] THERE IS NO ROOM WITH THAT NAME");
		return -1;
	}

	if(!(session->user->in_chat)) {
		SERVER_send_message(session->socket_descriptor, 
				"[SERVER] YOU ARE NOT ON CHAT");
		return -1;
	}

	char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
	strncpy(snd_buf, session->user->name, MAX_USER_NAME);
	strncat(snd_buf, " has left the room!", MAX_MESSAGE_SIZE);
	SERVER_room_broadcast(session, snd_buf);
	ROOM_kick_user(new_room, session->user);

	return 0;
}

// Broadcast message <buffer> to all users participating in <session>'s user
// room
int SERVER_room_broadcast(SESSION *session, char *buffer) {
	SESSION *session_ptr = NULL;
	// Invalid session
	if(session->user == NULL) {
		SERVER_send_message(session->socket_descriptor, 
				"[SERVER] PLEASE, SET YOUR NAME FIRST!");
		fprintf(stderr, "[SERVER] UNNAMED CLIENT!\n");

		pthread_mutex_unlock(&roomListMutex);
		return 0;
	}

	// User is not in chat (doesnt make sense)
	if(!session->user->in_chat) {
		pthread_mutex_unlock(&roomListMutex);
		return 0;
	}

	// Everything's allright
	ROOM *room = session->user->room;
	LIST *users = room->online_users;
	USER *user = (USER *) (users->node);

	pthread_mutex_lock(&roomListMutex);
	char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
	strncat(snd_buf, "[", sizeof("[")); strncat(snd_buf, room->name, MAX_ROOM_NAME);
	strncat(snd_buf, " ] ", sizeof(" ]"));
	strncat(snd_buf, session->user->name, strnlen(session->user->name,
				MAX_USER_NAME));
	strncat(snd_buf, " said: ", strlen(" said: "));
	strncat(snd_buf, buffer, strnlen(buffer, MAX_MESSAGE_SIZE));
	// Message broadcasting to room members
	while (users)
	{
		user = (USER *) (users->node);
		session_ptr = SERVER_get_session_by_user(user->name);
		if(user->in_chat)
			SERVER_send_message(session_ptr->socket_descriptor, snd_buf);
		users = users->next; 
	}

	pthread_mutex_unlock(&roomListMutex);
	return 0;
}

// TODO
int SERVER_send_whisper(SESSION *session, char *buffer) {
	return 0;
}

// Sends a help message to <session>, explaining possible commands
int SERVER_help(SESSION *session, char *buffer) {
	SERVER_send_message(session->socket_descriptor, HELP_STR);
	return 0;
}

// List, to <session>, the currently available rooms in the server
int SERVER_list(SESSION *session) {
	LIST *rooms = server->rooms;
	ROOM *check;
	char *snd_buf = malloc(sizeof(char) * MAX_ROOM_NAME * MAX_ROOMS);

	strncat(snd_buf, "Rooms:\n", strlen("Rooms: \n"));
	pthread_mutex_lock(&roomListMutex);
	while (rooms)
	{
		check = (ROOM *) (rooms->node);
		strncat(snd_buf, check->name, strnlen(check->name, MAX_ROOM_NAME));
		strcat(snd_buf, "\n");
		rooms = rooms->next;

	}
	pthread_mutex_unlock(&roomListMutex);
	SERVER_send_message(session->socket_descriptor, snd_buf);
	return 0;
}

// Ends a session
int SERVER_session_disconnect(SESSION *session) {
	SESSION_disconnect(session);
	return 0;
}

int SERVER_invalid_command(char *buffer) {
	return 1;
}

/*
 * Returns the index of the first available (disconnected) session
 */
int free_session_index(SESSION session_list[]) {
	int i = 0;
	pthread_mutex_lock(&userListMutex);
	while(session_list[i].valid != 0 && i < MAX_SESSIONS)
		i++;
	if(i == MAX_SESSIONS)
	{
		pthread_mutex_unlock(&userListMutex);
		return -1;
	}
	pthread_mutex_unlock(&userListMutex);
	return i;
}

/* 
 * Instantiates a new SERVER struct (returns a pointer to it, it must be freed
 * later
 */
SERVER *SERVER_new(short family, unsigned short port, USER *admin) {

	SERVER *server = (SERVER *) malloc(sizeof(SERVER));
	/* creates a socket with TCP protocol */
	server->socket_descriptor = socket(family, SOCK_STREAM, 0);
	if (server->socket_descriptor == -1) {
		fprintf(stderr, "ERROR: could not open server socket\n");
		return NULL;
	}

	/* server socket init */
	server->server_socket.sin_family = family;
	server->server_socket.sin_addr.s_addr = INADDR_ANY;
	// htons is for endian compatibility
	server->server_socket.sin_port = htons(port); 

	/* server socket binding */
	if(bind(server->socket_descriptor,
				(struct sockaddr *)(&server->server_socket),
				sizeof(server->server_socket)) < 0) {
		fprintf(stderr, "ERROR: could not bind server socket\n");	
		return NULL;
	}

	server->server_admin = admin;
	int i;
	for(i = 0; i < MAX_SESSIONS; i++) {
		SESSION_set(&server->sessions[i], AF_INET, PORT, SERVER_HOSTNAME,
				PORT, NULL); 
		char dfn[MAX_USER_NAME];
		memset(dfn, 0, sizeof(dfn));
		char dfnidx[2];
		dfnidx[0] = 65 + i;
		dfnidx[1] = '\0';
		strncat(dfn, "DEFUSER_", strlen("DEFUSER_"));
		strncat(dfn, dfnidx, strlen(dfnidx));
		USER_change_name(server->sessions[i].user, dfn);
	}

	server->rooms = NULL; //LIST_create(lobby);

	return server;
}

ROOM *SERVER_get_room_by_name(char *roomName)
{	
	LIST *rooms = server->rooms;
	ROOM *check;
	if (roomName == NULL)
	{
		fprintf(stderr,"ERROR: Null pointer to user name!");
		return NULL;
	}

	pthread_mutex_lock(&roomListMutex);
	while (rooms)
	{
		check = (ROOM *) (rooms->node);
		if (!strcmp(roomName, check->name))
		{	
			pthread_mutex_unlock(&roomListMutex);
			return check;
		}

		rooms = rooms->next;

	}
	pthread_mutex_unlock(&roomListMutex);
	return NULL;
}

SESSION *SERVER_get_session_by_user(char *userName)
{
	int i;
	SESSION *check;
	pthread_mutex_lock(&userListMutex);	
	for (i = 0; i < MAX_SESSIONS; i++)
	{
		check = &(server->sessions[i]);

		if(!strncmp(check->user->name, userName, MAX_USER_NAME)) {
			pthread_mutex_unlock(&userListMutex);
			return check;
		}

	}
	pthread_mutex_unlock(&userListMutex);
	return NULL;
}
