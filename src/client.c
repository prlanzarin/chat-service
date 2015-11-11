#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/session.h"
#include "../include/ui.h"
#include <unistd.h>
#include <pthread.h> 
#include <curses.h>
#include "../include/client.h"

SESSION *session;
int chatInRow = CHAT_INPUT_ROW;
int welcomeRow = WELCOME_ROW;
int in_chat = 0;
int choosingRoom;
int UI_MAXY2, UI_MAXX2;
pthread_mutex_t scr_mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *top; WINDOW *bottom;

pthread_t input_thread, listen_thread;

int main(int argc, char *argv[]) {
	// INIT TOP AND BOTTOM WINDOWS 
	initscr();		
	getmaxyx(stdscr, UI_MAXY2, UI_MAXX2);
	curs_set(2);
	top = newwin(UI_MAXY2-3, UI_MAXX2, 0, 0);
	bottom = newwin(3, UI_MAXX2, UI_MAXY2-3, 0);
	box(bottom, '|', '+'); 
	scrollok(top, TRUE);
	scrollok(bottom, TRUE);

	// Client session start 
	session = SESSION_new(AF_INET, PORT, SERVER_HOSTNAME, PORT, NULL);
	if(SESSION_connect(session) == -1)
	{
		printf ("ERROR: could not create session.");
		return -1;
	}
	// Listener
	if(pthread_create(&listen_thread, NULL, read_message, NULL) < 0) 
	{
		perror("ERROR: client message reading thread could not be created.");
		exit(EXIT_FAILURE);
	}
	// Sender
	if(pthread_create(&input_thread, NULL, send_message, NULL) < 0)
	{
		perror("ERROR: client message sending thread could not be created.");
		exit(EXIT_FAILURE);
	}

	pthread_join(listen_thread, NULL);
	pthread_join(input_thread, NULL);

	endwin();
	return 0; 
}

/*
 * Chat startup: reads user input and send it to the server
 * It's the client's main reading loop!
 */
void *send_message()
{
	char sendbuffer[MAX_MESSAGE_SIZE];
	memset(sendbuffer, 0, MAX_USER_COMMAND);	

	while(1) {

		memset(sendbuffer, 0, sizeof(sendbuffer));

		pthread_mutex_lock(&scr_mutex2);
		mvwprintw(bottom, chatInRow, CHAT_INPUT_COLUMN, "> ");
			pthread_mutex_unlock(&scr_mutex2);
		// Command parsing
		wgetnstr(bottom, sendbuffer, MAX_MESSAGE_SIZE-2);
		// Bottom refresh
		pthread_mutex_lock(&scr_mutex2);
		wclear(bottom);
		box(bottom, '|', '+'); 
		wprintw(bottom, "> ");
		wrefresh(bottom);
		pthread_mutex_unlock(&scr_mutex2);

		if (strstr(sendbuffer, QUIT) != NULL) {
			// just close everything 
			goto end;
		}
		// Send to server
		pthread_mutex_lock(&writeMutex);
		if(write_to_socket(session->socket_descriptor, sendbuffer) < 0)
			goto end;
		pthread_mutex_unlock(&writeMutex);

	}
end:
	endwin();			
	close(session->socket_descriptor);
	exit(0);
}

/* Sends a message <buffer> through socket <sock> */
int write_to_socket(int sock, char *buffer) {
	int m_size = 0; 
	m_size = strlen(buffer) + 1;
	if((write(sock, &m_size, sizeof(int))) < sizeof(int)) {
		fprintf(stderr, "ERROR: could no write on socket\n");
		return -1;
	}

	if(write(sock, buffer, m_size) < 0) {  //All the bytes have been sent 
		fprintf(stderr, "ERROR: could no write on socket %d\n", sock);
		return -1;
	}

	return 0;
}

/*
 * Connection Startup - Greetings & choose nickname
 * Then procceeds to server listening: it's the client's main listening loop
 */	
void *read_message()
{
	char buffer[MAX_MESSAGE_SIZE], *ptr;
	int m_size = 0; 
	while(1)
	{	
		memset(buffer,0,sizeof(buffer));				
		m_size = 0; 

		int rec = read(session->socket_descriptor, &m_size, sizeof(int));
		if(rec < 0)  // read failure
			goto end;	
		if(rec == 0) //end of connection
			goto end;

		ptr = buffer;
		while(m_size > 0) {
			rec = read(session->socket_descriptor, ptr, 
					m_size); 
			if(rec < 0)  // read failure
				goto end;	
			if(rec == 0) //end of connection
				goto end;
			ptr = ptr + rec;
			m_size = m_size - rec;
		}

		pthread_mutex_lock(&scr_mutex2);
		wprintw(top, "\n ");
		wprintw(top, buffer);
		wrefresh(top);
		pthread_mutex_unlock(&scr_mutex2);

	}
end:
	fprintf(stderr, "ERROR: could not read from server");
	endwin();
	exit(EXIT_FAILURE);

	pthread_exit(0);
}
