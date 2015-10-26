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

int main(int argc, char *argv[]) {
	int socket_descriptor;
	struct sockaddr_in server;
	struct hostent *serv_host;
	// socket creation
	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_descriptor == -1)
		printf("SOCKET COULD NO BE CREATED!\n");
	
	/*server.sin_addr.s_addr = inet_addr("74.125.235.20");
	server.sin_family = AF_INET;
	server.sin_port = htons(80);*/

	serv_host = gethostbyname("localhost"); //The "localhost" can be changed to get the name (or IP number) of the host from the command line . This is for testing purposes only
	server.sin_family = AF_INET;     
	server.sin_port = htons(PORT);    
	server.sin_addr = *((struct in_addr *)serv_host->h_addr);

	if(connect(socket_descriptor, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("CONNECTION ERROR\n");
		return 1;
	}
	printf("CONNECTED\n");

	return 0; 
}
