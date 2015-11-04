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
int chatRow = CHAT_OUTPUT_ROW;
int chatInRow = CHAT_INPUT_ROW;
int in_chat = 0;
int choosingRoom;
pthread_mutex_t choosingRoomMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t inChatMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scrMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t chatMutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *welcome;
WINDOW *top;
WINDOW *bottom;
pthread_t input_thread;
pthread_t read_thread;

int main(int argc, char *argv[]) {
	char buffer[256];
	
	
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
	
	//drawChat();

	pthread_join(read_thread, NULL);
	return 0; 
}

void drawWelcome() {
   	pthread_mutex_lock(&scrMutex);
	welcome = newwin(maxy, maxx, 0, 0);
	scrollok(welcome, TRUE);
	box(welcome, '|', '=');
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



void *read_message()
{
	int finish = 0;
	char buffer[256];
	
	//Connection Startup - Greetings & choose nickname
	while(!finish)
	{	
		memset(buffer,0,sizeof(buffer));				
		if(read(session->socket_descriptor, buffer, sizeof(buffer)) > 0);
		{	
			pthread_mutex_lock(&scrMutex);
			if (!strcmp(buffer,"wait"))
			{				
				mvwprintw(welcome, welcomeRow, WELCOME_COLUMN, "Finished startup\n");
				finish = 1;
			}
			else
			{				
				mvwprintw(welcome, welcomeRow, WELCOME_COLUMN, buffer);
				welcomeRow++;
				wrefresh(welcome);
				memset(buffer,0,sizeof(buffer));
			}			
			pthread_mutex_unlock(&scrMutex);
		}
	}

	//Show the available rooms
	chooseRoom();
	return NULL;
}

void getAvailableRooms()
{
	int finish = 0;
	int count = 0;
	char roomName[MAX_ROOM_NAME];
	int ack;
	int waitAck = 10;
	int finishAck = 11;
	int r;
	
	write(session->socket_descriptor, &waitAck, sizeof(int));
	while(finish == 0)
	{
		memset(roomName, 0, MAX_ROOM_NAME);	
		r = read(session->socket_descriptor, roomName, MAX_ROOM_NAME);
		if (r > 0)
		{
			if(!strcmp(roomName, "finish")) {
				write(session->socket_descriptor, &finishAck, sizeof(int));
				finish = 1;
				break;			
			}
			//Prints only the rooms with a name
			if(strcmp(roomName, " ")) {
				pthread_mutex_lock(&scrMutex);					
				mvwprintw(welcome, welcomeRow+2+count, 2, roomName);
				//fprintf(stderr, "\n\n\n\n\nali>=%s\n",roomName);				
				wrefresh(welcome);
				pthread_mutex_unlock(&scrMutex);
				ack = count;
				write(session->socket_descriptor, &ack, sizeof(int));
				count++;
			}
			
		}				
	}
	return;
}

void chooseRoom() {
	char *selectedRoom = malloc(sizeof(char)*MAX_ROOM_NAME);
	char userInput[MAX_USER_INPUT];
	char *userCommand = malloc(sizeof(char)*MAX_USER_COMMAND);
	char buffer[256];	
	int canCreateRoom;
	int canEnterRoom;

	pthread_mutex_lock(&choosingRoomMutex);
	choosingRoom = 1;
	pthread_mutex_unlock(&choosingRoomMutex);

	memset(selectedRoom, 0, MAX_ROOM_NAME);
	pthread_mutex_lock(&scrMutex);
	welcomeRow = WELCOME_ROW;
	pthread_mutex_unlock(&scrMutex);	
	drawWelcome();
	
	//The client is choosing a Room.
	while(choosingRoom == 1) {
		
		//writes the in the chat window
		pthread_mutex_lock(&scrMutex);
		mvwprintw(welcome, welcomeRow, 2, "Choose your room typing \\join <room_name>:");
		wrefresh(welcome);
		mvwprintw(welcome, welcomeRow+1, 2, "Available rooms:");
		wrefresh(welcome);
		pthread_mutex_unlock(&scrMutex);	
		getAvailableRooms();			

		memset(userInput, 0, MAX_USER_INPUT);
		memset(userCommand, 0, MAX_USER_COMMAND);	

		//gets the whole user input <command> + <arguments>
		mvwgetnstr(welcome, welcomeRow, 45, userInput, MAX_USER_INPUT);

		//sends the whole input to the server
		write(session->socket_descriptor, userInput, strlen(userInput));	
		//gets only the command from the input
		userCommand = strtok(userInput, " ");

		
		if (!strcmp(userCommand, "\\join"))
		{			
			read(session->socket_descriptor, &canEnterRoom, sizeof(int));
			selectedRoom = strtok(NULL, " ");
			if(canEnterRoom == 1)
			{
				pthread_mutex_lock(&choosingRoomMutex);
				choosingRoom = 0;
				pthread_mutex_unlock(&choosingRoomMutex);
				drawChat();
				pthread_mutex_lock(&inChatMutex);
				in_chat = 1;
				pthread_mutex_unlock(&inChatMutex);
				listenToMsgs();
			}
			else
			{
				clear_last_line();
				clear_command_input();
				pthread_mutex_lock(&scrMutex);	
				mvwprintw(welcome, 22, 2, "Room %s doesn't exist. Try another one", selectedRoom);
				wrefresh(welcome);			
				pthread_mutex_unlock(&scrMutex);
			
				//The command is invalid. Wait until the server refreshes the rooms list.
				if(read(session->socket_descriptor, buffer, sizeof(buffer)) > 0)
					if(!strcmp(buffer,"wait"))
						fprintf(stderr,"Received wait message.\n");
			}
		}
		else if (!strcmp(userCommand, "\\quit")) 
		{
			endwin();			
			close(session->socket_descriptor);
			pthread_mutex_lock(&choosingRoomMutex);
			choosingRoom = 0;
			pthread_mutex_unlock(&choosingRoomMutex);

		}
		else if (!strcmp(userCommand, "\\create"))
		{
			//Server responds if the room can be created
			read(session->socket_descriptor, &canCreateRoom, sizeof(int));
			clear_last_line();
			clear_command_input();
			pthread_mutex_lock(&scrMutex);
			if (!canCreateRoom)
				mvwprintw(welcome, 22, 2, "There was a problem creating the room %s.");
			else			
				mvwprintw(welcome, 22, 2, "Room %s created.", strtok(NULL, " "));
			wrefresh(welcome);
			pthread_mutex_unlock(&scrMutex);
			if(read(session->socket_descriptor, buffer, sizeof(buffer)) > 0)
				if(!strcmp(buffer,"wait"))
					fprintf(stderr,"Received wait message.\n");
		}
		else if (!strcmp(userCommand, "\\nick"))
		{
			read_message();
		}
		else {
			clear_last_line();
			clear_command_input();
			pthread_mutex_lock(&scrMutex);	
			mvwprintw(welcome, 22, 2, "Invalid command");
			wrefresh(welcome);			
			pthread_mutex_unlock(&scrMutex);
			
			//The command is invalid. Wait until the server refreshes the rooms list.
			if(read(session->socket_descriptor, buffer, sizeof(buffer)) > 0)
				if(!strcmp(buffer,"wait"))
					fprintf(stderr,"Received wait message.\n");			
		}
			    
	}
	
}

void clear_last_line()
{
	pthread_mutex_lock(&scrMutex);
	//clears last line of the screen
	mvwprintw(welcome, 22, 2, "                                                                            ");
	wrefresh(welcome);
	pthread_mutex_unlock(&scrMutex);
	return;
}
void clear_command_input()
{
	pthread_mutex_lock(&scrMutex);
	mvwprintw(welcome, welcomeRow, 45, "                           ");
	wrefresh(welcome);
	pthread_mutex_unlock(&scrMutex);
}


void listenToMsgs() {
	int bufferSize = maxx-4;
	char *buffer = malloc(bufferSize);
	int waitAck = 1;

	if(pthread_create(&input_thread, NULL, send_message, NULL) < 0) 
	{
		perror("ERROR: client message sending thread could not be created.");
		exit(EXIT_FAILURE);
	}

	while(in_chat) {
	
		memset(buffer, 0, bufferSize);
		pthread_mutex_lock(&scrMutex);
		wrefresh(top);
		wrefresh(bottom);
		pthread_mutex_unlock(&scrMutex);

		//receives message from the server
		read(session->socket_descriptor, buffer, bufferSize);		

		if (!strcmp(buffer, "wait"))
		{
			write(session->socket_descriptor, &waitAck, sizeof(int));			
			break;
		}		

        	pthread_mutex_lock(&scrMutex);
		//writes the message received in the terminal
		mvwprintw(top, chatRow, 3, buffer);
		wrefresh(top);
        	pthread_mutex_unlock(&scrMutex);
        
		pthread_mutex_lock(&chatMutex);
		
		//scrolls the top if the line number exceeds height
		if(chatRow != (maxy/2 - 2))
			chatRow++;
		else
			scroll(top);
		pthread_mutex_unlock(&chatMutex);
	}
	
	pthread_exit(NULL);

}

void *send_message()
{
	char buffer[256];
	int msg_ack = 0;
	int left_chat = 0;
	char userInput[MAX_USER_INPUT];
	char *userCommand = malloc(sizeof(char)*MAX_USER_COMMAND);
	memset(userInput, 0, MAX_USER_INPUT);
	memset(userCommand, 0, MAX_USER_COMMAND);	

	while(in_chat) {
		memset(buffer,0,sizeof(buffer));				
		pthread_mutex_lock(&scrMutex);
		mvwprintw(bottom, chatInRow, CHAT_INPUT_COLUMN, "> ");
		pthread_mutex_unlock(&scrMutex);
		
		wgetnstr(bottom, buffer, MAX_MESSAGE_SIZE - 2);
		//sends the whole input to the server
		write(session->socket_descriptor, userInput, strlen(userInput));	
		//gets only the command from the input
		userCommand = strtok(buffer, " ");
		if (!strcmp(userCommand, "\\send"))
		{
			//Server responds to room broadcast
			read(session->socket_descriptor, &msg_ack, sizeof(int));
			if(!msg_ack)
				fprintf(stderr,"Failed to send message.\n");

			pthread_mutex_lock(&scrMutex);
			wrefresh(bottom);
			pthread_mutex_unlock(&scrMutex);
			if(read(session->socket_descriptor, buffer, sizeof(buffer)) > 0)
				if(!strcmp(buffer,"wait"))
					fprintf(stderr,"Received wait message.\n");
		}
		else if (!strcmp(userCommand, "\\leave"))
		{
			//Server responds to leave room request 
			read(session->socket_descriptor, &left_chat, sizeof(int));
			if(!left_chat)
				fprintf(stderr, "Failed to leave. Try again\n");
			pthread_mutex_lock(&inChatMutex);
			in_chat = 0;
			pthread_mutex_unlock(&inChatMutex);
			goto end;	
		}
		pthread_mutex_lock(&scrMutex);
		wclear(bottom);
		wrefresh(bottom);
		pthread_mutex_unlock(&scrMutex);

	}
	end:
	free(userCommand);
	pthread_exit(0);
}
