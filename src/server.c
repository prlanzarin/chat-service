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
int curr_session;

int main(int argc , char *argv[])
{
	char *message;

	server = SERVER_new(AF_INET, PORT, NULL);
	curr_session = free_session_index(server->sessions);	

	// server stays listening on its socket for new client connections
	listen(server->socket_descriptor, BACKLOG_CONNECTIONS);

	// connection capture
	puts("Waiting for incoming connections...");

	while(curr_session != -1 && 
			(server->sessions[curr_session].socket_descriptor =
			 accept(server->socket_descriptor, 
				 (struct sockaddr *) &server->sessions[curr_session].client_socket,
				 (socklen_t*) &SOCKADDR_IN_SIZE))) {
		puts("Connection accepted");
		server->sessions[curr_session].valid = 1;
		// reply to the client
		message = "Hello Client, you'll be handled properly"; 
		write(server->sessions[curr_session].socket_descriptor, 
				message, strlen(message));
		pthread_t sniffer_thread;

		if(pthread_create(&sniffer_thread, NULL, connection_handler, 
					(void*)&server->sessions[curr_session].socket_descriptor) < 0) {
			perror("ERROR: new server thread could not be created");
			return 1;
		}

		// now join the thread , so that we dont terminate before the thread
		pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
		curr_session = free_session_index(server->sessions);	
	}

	return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;

	char *message;

	//Send some messages to the client
	message = "Greetings! I am your connection handler\n";
	write(sock , message , strlen(message));

	message = "Its my duty to communicate with you";
	write(sock , message , strlen(message));

	//Free the socket pointer
	free(socket_desc);

	return 0;
}

int free_session_index(SESSION session_list[]) {
	int i = 0;
	while(session_list[i].valid != 0 && i < MAX_SESSIONS)
		i++;
	if(i == MAX_SESSIONS)
		return -1;
	return i;
}

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
		return NULL;;
	}
	puts("SERVER SOCKET BINDING OK");

	server->server_admin = admin;
	int i;
	for(i = 0; i < MAX_SESSIONS; i++)
		SESSION_set(&server->sessions[i], AF_INET, PORT, NULL); 

	return server;
}
