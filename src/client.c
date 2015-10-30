#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/client.h"
#include "../include/session.h"
#include <unistd.h>
#include <pthread.h> 

SESSION *session;

int main(int argc, char *argv[]) {
	char buffer[256];
	pthread_t read_thread;

	session = SESSION_new(AF_INET, PORT, SERVER_HOSTNAME, PORT, NULL);
	if(SESSION_connect(session) == -1)
	{
		printf ("ERRROR: could not create session.");
		return -1;
	}
	if(pthread_create(&read_thread, NULL, read_message, NULL) < 0) 
	{
		perror("ERROR: client message reading thread could not be created.");
		exit(EXIT_FAILURE);
	}
	fgets(buffer, sizeof(buffer), stdin);
	write(session->socket_descriptor, buffer, sizeof(buffer));
	pthread_join(read_thread, NULL);
	return 0; 
}

void *read_message()
{
	char buffer[256];
	while(1)
	{
		if(read(session->socket_descriptor, buffer, sizeof(buffer)) > 0);		
		   puts(buffer);
	}
}

