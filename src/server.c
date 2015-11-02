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
int curr_session = -1;

int main(int argc , char *argv[])
{
	char *message;
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
		LIST_add_room(&(server->rooms), &testrooms[i]);
	}

	curr_session = free_session_index(server->sessions);	

	// server stays listening on its socket for new client connections
	listen(server->socket_descriptor, BACKLOG_CONNECTIONS);

	// connection capture
	puts("Waiting for incoming connections...");
	s_ptr = &server->sessions[0];
	while(curr_session != -1 && (s_ptr[curr_session].socket_descriptor =
				accept(server->socket_descriptor, 
					(struct sockaddr *) &s_ptr[curr_session].client_socket,
					(socklen_t*) &SOCKADDR_IN_SIZE))) {

		printf("FREE SESSION FOUND: %d\n", curr_session);
		s_ptr[curr_session].valid = 1;
		// reply to the client
		message = "	Hello Client, you'll be handled properly\n"; 
		write(s_ptr[curr_session].socket_descriptor, 
				message, strlen(message));

		if(pthread_create(&s_ptr[curr_session].session_thread, NULL, 
					connection_handler, 
					(void*)&s_ptr[curr_session].socket_descriptor) < 0) {
			perror("ERROR: new server thread could not be created");
			exit(EXIT_FAILURE);
		}

		

		// now join the thread , so that we dont terminate before the thread
		pthread_join(s_ptr[curr_session].session_thread, NULL);
		puts("Handler assigned");
		curr_session = free_session_index(server->sessions);	
	}
	return 0;
}

/*
 * This will handle connection for each client
 */
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	char buffer[256];
	char clientNickname[MAX_USER_NAME];

	char *message;
	

	//Send some messages to the client
	message = "	Greetings! I am your connection handler\n";
	write(sock, message, strlen(message));

	message = "	Its my duty to communicate with you\n";
	write(sock, message, strlen(message));

	message = "	Type your nickname:";
	write(sock, message, strlen(message));

	//Receive the chosen nickname
	if(read(sock, buffer, sizeof(buffer)) > 0)
		printf ("	The chosen nickname is %s\n",buffer);
		
	//Send the available rooms to the client
	SERVER_show_rooms(*((int*)socket_desc));

	strncpy(clientNickname,buffer,sizeof(clientNickname));
	SERVER_process_user_cmd(*((int*)socket_desc), clientNickname);

	//Free the socket pointer
	printf("CLOSING SESSION %d\n", sock);
	return 0;
} 
/*
 *
 * Returns the index of the first available (disconnected) session
 */
int free_session_index(SESSION session_list[]) {
	int i = 0;
	while(session_list[i].valid != 0 && i < MAX_SESSIONS)
		i++;
	if(i == MAX_SESSIONS)
		return -1;
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

	return server;
}


void SERVER_show_rooms(int socket)
{
	int ack;
	char waitMessage[5] = "wait";
	char finishMessage[7] = "finish";
	int waitAck;
	int finishAck;
	LIST *rooms = server->rooms;
	ROOM *room;

	fprintf(stderr, "Sending available rooms to the client %d\n",socket);

	//before showing the rooms, server must inform the listen thread in the client
	write(socket, waitMessage, strlen(waitMessage));
	fprintf(stderr, "Sent waitMessage: %s\n", waitMessage);
	read(socket, &waitAck, sizeof(int));
	fprintf(stderr, "Received waitAck %d\n", waitAck);

	pthread_mutex_lock(&roomListMutex);
	while (rooms)
	{
		room = (ROOM *) (rooms->node);
		write(socket, room->name, strlen(room->name));
		read(socket, &ack, sizeof(int));	
		fprintf(stderr, "Received roomAck %d\n",ack);
		rooms = rooms->next;		
	}
	pthread_mutex_unlock(&roomListMutex);

	fprintf(stderr, "Finished sending available rooms.\n");
	fprintf(stderr, "Sending finishMessage: %s\n", finishMessage);
	write(socket, finishMessage, strlen(finishMessage));
	read(socket, &finishAck, sizeof(int));
	fprintf(stderr, "Received finishAck %d\n", finishAck);
}

void SERVER_process_user_cmd (int socket, char *clientNickname)
{
	fprintf(stderr,"Waiting for client %s's commands...\n",clientNickname);
	while(1) {
	}
}
