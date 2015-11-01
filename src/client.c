#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/client.h"
#include "../include/session.h"
#include <unistd.h>
#include <pthread.h> 
#include <curses.h>

#define WELCOME_ROW 1
#define WELCOME_COLUMN 2
#define WELCOME_INPUT_ROW 2
#define WELCOME_INPUT_COLUMN 21
#define CHAT_OUTPUT_ROW 1
#define CHAT_OUTPUT_COLUMN 2
#define CHAT_INPUT_ROW 1
#define CHAT_INPUT_COLUMN 2

SESSION *session;
int maxx, maxy; //screen dimensions
int welcomeRow = WELCOME_ROW;
pthread_mutex_t scrMutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *welcome;
WINDOW *top;
WINDOW *bottom;

int main(int argc, char *argv[]) {
	char buffer[256];
	pthread_t read_thread;
	
	
	initscr();		
	getmaxyx(stdscr, maxy, maxx);
	drawWelcome();	
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

	

	mvwgetnstr(welcome, WELCOME_INPUT_ROW, WELCOME_INPUT_COLUMN, buffer, 20);
	write(session->socket_descriptor, buffer, sizeof(buffer));
	
	
	chooseRoom();
	
	drawChat();


	
	pthread_join(read_thread, NULL);
	return 0; 
}

void drawWelcome() {
	welcomeRow = WELCOME_ROW;
    	pthread_mutex_lock(&scrMutex);
	welcome = newwin(maxy, maxx, 0, 0);
	scrollok(welcome, TRUE);
	
	box(welcome, '|', '+');
	wsetscrreg(welcome, 1, maxy-2);
	wrefresh(welcome);
	pthread_mutex_unlock(&scrMutex);
}

void drawChat() {
    pthread_mutex_lock(&scrMutex);
    top = newwin(maxy/2, maxx, 0, 0);
    bottom = newwin(maxy/2, maxx, maxy/2, 0);
    scrollok(top, TRUE);
    scrollok(bottom, TRUE);
    box(top, '|', '+');
    box(bottom, '|', '+');

    wsetscrreg(top, 1, maxy/2-2);
    wsetscrreg(bottom, 1, maxy/2-2);
    wrefresh(top);
    wrefresh(bottom);
    pthread_mutex_unlock(&scrMutex);
}

void chooseRoom()
{
}

void *read_message()
{
	char buffer[256];
	while(1)
	{
		if(read(session->socket_descriptor, buffer, sizeof(buffer)) > 0);
		{		
		   mvwprintw(welcome, welcomeRow, WELCOME_COLUMN, buffer);
		   welcomeRow++;
		   wrefresh(welcome);
		}
	}
}

