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
	int i;
	ROOM testrooms[10];	

	if((server = SERVER_new(AF_INET, PORT, NULL)) == NULL) {
		fprintf(stderr, "ERROR: server creation failed.\n");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < 10; i++)
	{
		testrooms[i].name[0] = 'a'+i;
		testrooms[i].name[1] = '\0';
		testrooms[i].online_users = NULL;
		server->rooms =	LIST_push(server->rooms, &testrooms[i]);
	}

	curr_session = free_session_index(server->sessions);	

	// server stays listening on its socket for new client connections
	listen(server->socket_descriptor, BACKLOG_CONNECTIONS);

	// connection capture
	puts("Waiting for incoming connections...");
	s_ptr = &server->sessions[0];
	while(1) {
		if((s_ptr[curr_session].socket_descriptor =
					accept(server->socket_descriptor, 
						(struct sockaddr *) &s_ptr[curr_session].client_socket, 
						(socklen_t*) &SOCKADDR_IN_SIZE)) < 0)
			fprintf(stderr, "ERROR: connection could not be accepted\n");


		if(curr_session != -1) 
			printf("FREE SESSION FOUND: %d\n", curr_session);
		else { 
			//send_message to client of no free sessions
			SERVER_send_message(s_ptr[curr_session].socket_descriptor,
				       	"NO FREE SESSIONS!");
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
		puts("Handler assigned");
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
	size_t msg_size = 0;
	char buffer[MAX_MESSAGE_SIZE];
	char *message, *ptr;

	//Send some messages to the client
	message = "	Hello! Type \\nick <name> to enter your name:";
	if (SERVER_send_message(sock, message) < 0)
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

		if (strstr(buffer, USER_SEND_MESSAGE_TO_ROOM))
			SERVER_room_broadcast(session, 
					buffer + strlen(USER_SEND_MESSAGE_TO_ROOM));

		else if (strstr(buffer, USER_LEAVE_ROOM))
			SERVER_leave_room(session, buffer + strlen(USER_LEAVE_ROOM));

		else if (strstr(buffer, USER_JOIN_ROOM))
			SERVER_join_room(session, buffer + strlen(USER_JOIN_ROOM));

		else if (strstr(buffer, QUIT)) 
			SERVER_session_disconnect(session);

		else if (strstr(buffer, ROOM_CREATION))
			SERVER_create_room(session, buffer + strlen(ROOM_CREATION));

		else if (strstr(buffer, USER_NICKNAME))
			SERVER_new_user(session, buffer + strlen(USER_NICKNAME));

		else if (strstr(buffer, ROOM_LISTING))
			SERVER_list(session, buffer + strlen(ROOM_LISTING));

		else if (strstr(buffer, HELP))
			SERVER_help(session, buffer + strlen(HELP));

		else if (strstr(buffer, USER_SEND_PRIVATE_MESSAGE)) 
			SERVER_send_whisper(session, 
					buffer + strlen(USER_SEND_PRIVATE_MESSAGE));

		else if (SERVER_invalid_command(buffer))
			SERVER_send_message(sock, "INVALID COMMAND!");

		else 
			SERVER_send_message(sock, "INVALID COMMAND!");
	}

	SERVER_session_disconnect(session);
	pthread_exit(0);
} 

int SERVER_new_user(SESSION *session, char *buffer) {
	char *snd_buf = malloc(sizeof(MAX_MESSAGE_SIZE));
	if(USER_change_name(session->user, buffer) < 0)
		SERVER_send_message(session->socket_descriptor, "NAME TOO LARGE");
	else {
		strncpy(snd_buf, "Your new nick is ", MAX_MESSAGE_SIZE);
		strncat(snd_buf, session->user->name, MAX_MESSAGE_SIZE);
		SERVER_send_message(session->socket_descriptor, snd_buf);
	}	
	free(snd_buf);
	return 0;
}

int SERVER_create_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);
	pthread_mutex_lock(&roomListMutex);
	if (new_room == NULL)
	{
		ROOM *new_room = ROOM_create(buffer, session->user);	
		if(new_room == NULL) {
			SERVER_send_message(session->socket_descriptor, "NAME TOO LARGE");
			return -1;
		}
		server->rooms = LIST_push(server->rooms, new_room);
		pthread_mutex_unlock(&roomListMutex);
		return 0;
	}
	else
	{
		fprintf(stderr, "ERROR: room name already exists");
		SERVER_send_message(session->socket_descriptor, "ROOM ALREADY EXISTS");
		pthread_mutex_unlock(&roomListMutex);
		return -1;
	}
	return 0;
}

int SERVER_join_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);

	if(new_room == NULL) {
		SERVER_send_message(session->socket_descriptor, 
				"THERE IS NO ROOM WITH THAT NAME");
		return -1;
	}

	if(session->user->in_chat) {
		SERVER_send_message(session->socket_descriptor, 
				"ALREADY ON CHAT");
		return -1;
	}

	ROOM_add_user(new_room, session->user);
	char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
	strncpy(snd_buf, session->user->name, MAX_USER_NAME);
	strncat(snd_buf, " has joined the room!", MAX_MESSAGE_SIZE);
	SERVER_room_broadcast(session, snd_buf);
	free(snd_buf);

	return 0;
}

int SERVER_leave_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);

	if(new_room == NULL) {
		SERVER_send_message(session->socket_descriptor, 
				"THERE IS NO ROOM WITH THAT NAME");
		return -1;
	}

	if(!(session->user->in_chat)) {
		SERVER_send_message(session->socket_descriptor, 
				"ALREADY ON CHAT");
		return -1;
	}

	ROOM_kick_user(new_room, session->user);
	char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
	strncpy(snd_buf, session->user->name, MAX_USER_NAME);
	strncat(snd_buf, " has left the room!", MAX_MESSAGE_SIZE);
	SERVER_room_broadcast(session, snd_buf);
	free(snd_buf);

	return 0;
}

int SERVER_room_broadcast(SESSION *session, char *buffer) {
	SESSION *session_ptr = NULL;
	ROOM *room = session->user->room;
	LIST *users = room->online_users;
	USER *user = (USER *) (users->node);
	pthread_mutex_lock(&roomListMutex);
	while (users)
	{
		user = (USER *) (users->node);
		session_ptr = SERVER_get_session_by_user(user->name);
		SERVER_send_message(session_ptr->socket_descriptor, buffer);
		users = users->next; 
	}
	pthread_mutex_unlock(&roomListMutex);
	return 0;
}

int SERVER_send_whisper(SESSION *session, char *buffer) {
	return 0;
}

int SERVER_help(SESSION *session, char *buffer) {
	SERVER_send_message(session->socket_descriptor, HELP_STR);
	return 0;
}

int SERVER_list(SESSION *session, char *buffer) {
	return 0;
}

int SERVER_session_disconnect(SESSION *session) {
	return 0;
}

int SERVER_send_message(int socket, char *buffer) {
	int m_size = 0; 

	pthread_mutex_lock(&socketw_mutex);
	m_size = strlen(buffer) + 1;
	// sending message size to client
	if(write(socket, &m_size, sizeof(int)) < sizeof(int)) {
		fprintf(stderr, "ERROR: could no write on socket %d\n", socket);
		return -1;
	}
	// sending actual message to client
	if(write(socket, buffer, strlen(buffer) + 1) > 0) 
		return 0;
	else {
		fprintf(stderr, "ERROR: could no write on socket %d\n", socket);
		return -1;
	}
	pthread_mutex_unlock(&socketw_mutex);

}

int SERVER_invalid_command(char *buffer) {
	return 0;
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
	puts("SERVER SOCKET CREATION OK");

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
	puts("SERVER SOCKET BINDING OK");

	server->server_admin = admin;
	int i;
	for(i = 0; i < MAX_SESSIONS; i++)
		SESSION_set(&server->sessions[i], AF_INET, PORT, SERVER_HOSTNAME,
				PORT, NULL); 

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
		if (!strcmp (roomName, check->name))
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
