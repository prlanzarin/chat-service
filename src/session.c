#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "../include/session.h"

SESSION *SESSION_new(short family, unsigned short port, char *hostname, 
		unsigned short server_port, USER *user)  {

	SESSION *session = (SESSION *)malloc(sizeof(SESSION));
	if(SESSION_set(session, family, port, hostname, server_port,
				user) == -1){
		free(session);
		session = NULL;
	}
	return session;
}

int SESSION_set(SESSION *session, short family, unsigned short port, 
		char *hostname, unsigned short server_port, USER *user) {

	struct hostent *s_host;

	session->client_socket.sin_family = family;
	session->client_socket.sin_addr.s_addr = INADDR_ANY;
	session->client_socket.sin_port = htons(port); // htons -> endianess
	session->valid = 0;
	session->user = (USER *)malloc(sizeof(USER));
	session->user->set = 0;
	session->user->in_chat = 0;

	/* Server adressing.
	 * The "localhost" can be changed to get the name (or IP number) of the
	 * host from the command line . This is for testing purposes only */
	s_host = gethostbyname(hostname); session->server.sin_family = AF_INET;     
	session->server.sin_port = htons(server_port);    
	session->server.sin_addr = *((struct in_addr *)s_host->h_addr);

	return 0;
}


int SESSION_connect(SESSION *session) {

	session->socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	/* creates a socket with TCP protocol */
	if (session->socket_descriptor == -1) {
		fprintf(stderr, "ERROR: could not open client socket\n");
		return -1;
	}
	//printf("SESSION SOCKET %d WAS OPENED.\n", session->socket_descriptor);

	if(connect(session->socket_descriptor, 
				(struct sockaddr *)&session->server,
				sizeof(session->server)) < 0) {
		printf("CONNECTION ERROR\n");
		return -1;
	}
	session->valid = 1;

	return 0;
}

int SESSION_disconnect(SESSION *session) {
	close(session->socket_descriptor);
	session->socket_descriptor = -1;
	session->valid = 0;
	USER_quit(session->user);
	return 0;
}
