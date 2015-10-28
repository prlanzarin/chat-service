#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/client.h"
#include "../include/session.h"

struct sockaddr_in server;
struct hostent serv_host;
SESSION *session;

int main(int argc, char *argv[]) {

	session = SESSION_new(AF_INET, PORT, NULL);
	SESSION_connect(session, SERVER_HOSTNAME, PORT, &server, 
			&serv_host);

	return 0; 
}


