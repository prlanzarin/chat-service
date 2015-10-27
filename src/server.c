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

SERVER server;

int main(int argc , char *argv[])
{
	int new_socket, *new_sock;
	//struct sockaddr_in server, client;
	struct sockaddr_in client;
	char *message;

	SERVER_init(&server, AF_INET, PORT, NULL);

	// server stays listening on its socket for new client connections
	listen(server.socket_descriptor, BACKLOG_CONNECTIONS);

	// connection capture
	puts("Waiting for incoming connections...");

	while((new_socket = accept(server.socket_descriptor, 
					(struct sockaddr *) &client, 
					(socklen_t*) &SOCKADDR_IN_SIZE))) {
		puts("Connection accepted");
		// reply to the client
		message = "Hello Client, you'll be handled properly"; 
		write(new_socket, message, strlen(message));
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = new_socket;

		if(pthread_create(&sniffer_thread, NULL, connection_handler, 
					(void*)new_sock) < 0) {
			perror("ERROR: new server thread could not be created");
			return 1;
		}

		// now join the thread , so that we dont terminate before the thread
		pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}

	if (new_socket < 0) {
		perror("accept failed");
		return 1;
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

int SERVER_new(SERVER *server, short family, unsigned short port, 
		USER *admin) {

	/* creates a socket with TCP protocol */
	server->socket_descriptor = socket(family, SOCK_STREAM, 0);
	if (server->socket_descriptor == -1) {
		fprintf(stderr, "ERROR: could not open server socket\n");
		return -1;
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
		return -1;
	}
	puts("SERVER SOCKET BINDING OK");
	
	server->server_admin = admin;
	return 0;
}
