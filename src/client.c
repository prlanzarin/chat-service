#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../include/client.h"

/*
struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET, AF_INET6
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};
 
struct in_addr {
    unsigned long s_addr;          // load with inet_pton()
};
 
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};
*/

struct sockaddr_in server;
struct hostent *serv_host;
CLIENT session;

int main(int argc, char *argv[]) {


	int socket_descriptor;
	struct sockaddr_in server;
	struct hostent *serv_host;
	CLIENT_new_session(&session, SERVER_HOSTNAME, AF_INET, PORT, NULL);
	// socket creation

	printf("CONNECTED\n");

	return 0; 
}

int CLIENT_new_session(CLIENT *session, char *hostname, short family, 
		unsigned short port, USER *user) {	
		
	/* creates a socket with TCP protocol */
	session->socket_descriptor = socket(family, SOCK_STREAM, 0);
	if (session->socket_descriptor == -1) {
		fprintf(stderr, "ERROR: could not open client socket\n");
		return -1;
	}
	puts("CLIENT SOCKET CREATION OK");
	
	session->client_socket.sin_family = family;
	session->client_socket.sin_addr.s_addr = INADDR_ANY;
	session->client_socket.sin_port = htons(port); // htons -> endianess
	
	/*The "localhost" can be changed to get the name (or IP number) of the
	 * host from the command line . This is for testing purposes only */
	serv_host = gethostbyname(hostname); server.sin_family = AF_INET;     
	server.sin_port = htons(port);    
	server.sin_addr = *((struct in_addr *)serv_host->h_addr);

	if(connect(session->socket_descriptor, 
				(struct sockaddr *)&server, 
				sizeof(server)) < 0) {
		printf("CONNECTION ERROR\n");
		return -1;
	}

	session->user = user;
	printf("CONNECTED\n");
	return 0;
}
