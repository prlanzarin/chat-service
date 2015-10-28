#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/session.h"

SESSION *SESSION_new(short family, unsigned short port, USER *user) {	
	SESSION *session = (SESSION *)malloc(sizeof(SESSION));
	if(SESSION_set(session, family, port, user) == -1){
		free(session);
		session = NULL;
	}
	return session;
}

int SESSION_set(SESSION *session, short family, unsigned short port, USER *user) {	
	/* creates a socket with TCP protocol */
	session->socket_descriptor = socket(family, SOCK_STREAM, 0);
	if (session->socket_descriptor == -1) {
		fprintf(stderr, "ERROR: could not open client socket\n");
		return -1;
	}
	
	session->client_socket.sin_family = family;
	session->client_socket.sin_addr.s_addr = INADDR_ANY;
	session->client_socket.sin_port = htons(port); // htons -> endianess
	session->valid = 0;
	session->user = user;
	return 0;
}


int SESSION_connect(SESSION *session, char *hostname, 
		unsigned short server_port, struct sockaddr_in *server_socket,
	       	struct hostent *s_host) {

	/*The "localhost" can be changed to get the name (or IP number) of the
	 * host from the command line . This is for testing purposes only */
	s_host = gethostbyname(hostname); server_socket->sin_family = AF_INET;     
	server_socket->sin_port = htons(server_port);    
	server_socket->sin_addr = *((struct in_addr *)s_host->h_addr);

	if(connect(session->socket_descriptor, 
				(struct sockaddr *)&server_socket, 
				sizeof(server_socket)) < 0) {
		printf("CONNECTION ERROR\n");
		return -1;
	}
	session->valid = 1;

	return 0;
}

