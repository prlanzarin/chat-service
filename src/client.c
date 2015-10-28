#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/client.h"
#include "../include/session.h"

SESSION *session;

int main(int argc, char *argv[]) {

	session = SESSION_new(AF_INET, PORT, SERVER_HOSTNAME, PORT, NULL);
	SESSION_connect(session);

	return 0; 
}


