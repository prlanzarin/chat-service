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
int chatRow = CHAT_OUTPUT_ROW;
int chatInRow = CHAT_INPUT_ROW;
int welcomeRow = WELCOME_ROW;
int in_chat = 0;
int choosingRoom;
pthread_mutex_t scrMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t chatMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *top; WINDOW *bottom;
WINDOW *welcome;

pthread_t input_thread, listen_thread;

int main(int argc, char *argv[]) {
	UI_init(top, bottom);

	// Client session start 
	session = SESSION_new(AF_INET, PORT, SERVER_HOSTNAME, PORT, NULL);
	if(SESSION_connect(session) == -1)
	{
		printf ("ERRROR: could not create session.");
		return -1;
	}

	if(pthread_create(&listen_thread, NULL, read_message, NULL) < 0) 
	{
		perror("ERROR: client message reading thread could not be created.");
		exit(EXIT_FAILURE);
	}

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
		memset(sendbuffer, 0, MAX_MESSAGE_SIZE);
		UI_read_from(bottom, sendbuffer, chatInRow, CHAT_INPUT_COLUMN,
				MAX_MESSAGE_SIZE);

		if (!strstr(sendbuffer, "\\send"))
		{
			//Server responds to room broadcast
			pthread_mutex_lock(&writeMutex);
			write(session->socket_descriptor, sendbuffer, strlen(sendbuffer)+1);
			pthread_mutex_unlock(&writeMutex);
			UI_redraw_window(bottom, '|', '+');
		}
		else if (!strstr(sendbuffer, "\\leave"))
		{
			//Server responds to leave room request 
			pthread_mutex_lock(&writeMutex);
			write(session->socket_descriptor, sendbuffer, strlen(sendbuffer)+1);
			pthread_mutex_unlock(&writeMutex);
			UI_redraw_window(top, '|', '+');
			UI_redraw_window(bottom, '|', '+');
		}
		else if (!strstr(sendbuffer, "\\join"))
		{			
			pthread_mutex_lock(&writeMutex);
			write(session->socket_descriptor, sendbuffer, strlen(sendbuffer)+1); 
			pthread_mutex_unlock(&writeMutex);

		}
		else if (!strstr(sendbuffer, "\\quit")) 
		{
			// just close everything pls
			endwin();			
			close(session->socket_descriptor);
			exit(1);

		}
		else if (!strstr(sendbuffer, "\\create"))
		{
			//sends the whole input to the server
			pthread_mutex_lock(&writeMutex);
			write(session->socket_descriptor, sendbuffer, strlen(sendbuffer)+1);	
			pthread_mutex_unlock(&writeMutex);
		}
		else if (!strstr(sendbuffer, "\\nick"))
		{
			clear_command_input();
			pthread_mutex_lock(&writeMutex);			
			write(session->socket_descriptor, sendbuffer, strlen(sendbuffer)+1);	
			pthread_mutex_unlock(&writeMutex);
		}
		else if (!strstr(sendbuffer, "\\ls")) {

		}
		else if (!strstr(sendbuffer, "\\help")) {

		}
		else {
			//Redraws the window
			UI_redraw_window(top, '|', '+');
			UI_redraw_window(bottom, '|', '+');
		}
	}

	pthread_exit(0);
}



/*
 * Connection Startup - Greetings & choose nickname
 * Then procceeds to server listening: it's the client's main listening loop
 */	
void *read_message()
{
	char buffer[MAX_MESSAGE_SIZE];

	while(1)
	{	
		memset(buffer,0,sizeof(buffer));				
		if(read(session->socket_descriptor, buffer, MAX_MESSAGE_SIZE) > 0);
		{	
			//writes the message received in the terminal
			UI_write_on_window(top, buffer, chatRow, 3);
			//scrolls the top if the line number exceeds height
			pthread_mutex_lock(&chatMutex);
			if(chatRow != (UI_MAXY/2 - 2))
				chatRow++;
			else
				scroll(top);
			pthread_mutex_unlock(&chatMutex);
		}
	}
	pthread_exit(0);
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

	pthread_mutex_lock(&writeMutex);
	write(session->socket_descriptor, &waitAck, sizeof(int));
	pthread_mutex_unlock(&writeMutex);

	while(finish == 0)
	{
		memset(roomName, 0, MAX_ROOM_NAME);	
		r = read(session->socket_descriptor, roomName, MAX_ROOM_NAME);
		if (r > 0)
		{
			if(!strcmp(roomName, "finish")) {
				pthread_mutex_lock(&writeMutex);
				write(session->socket_descriptor, &finishAck, sizeof(int));
				pthread_mutex_unlock(&writeMutex);
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
				pthread_mutex_lock(&writeMutex);
				write(session->socket_descriptor, &ack, sizeof(int));
				pthread_mutex_unlock(&writeMutex);
				count++;
			}

		}				
	}
	return;
}
void clear_command_input()
{
	pthread_mutex_lock(&scrMutex);
	mvwprintw(bottom, chatInRow, 45, "                           ");
	wrefresh(bottom);
	pthread_mutex_unlock(&scrMutex);
}

/*
void clear_last_line()
{
	pthread_mutex_lock(&scrMutex);
	//clears last line of the screen
	mvwprintw(welcome, 22, 2, "                                                                            ");
	wrefresh(welcome);
	pthread_mutex_unlock(&scrMutex);
	return;
}


void clear_chat_input()
{
	pthread_mutex_lock(&scrMutex);
	mvwprintw(welcome, welcomeRow, 45, "                           ");
	wrefresh(welcome);
	pthread_mutex_unlock(&scrMutex);

}
*/
/*
   void chooseRoom() {
   char *selectedRoom = malloc(sizeof(char)*MAX_ROOM_NAME);
   char userInput[MAX_USER_INPUT];
   char sendbuffer[MAX_USER_INPUT + 256];
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
//drawWelcome();
UI_redraw_window(welcome, '|', '=');

while(1) {
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
memset(sendbuffer, 0, MAX_USER_INPUT + 256);	
fflush(stdin);
//gets the whole user input <command> + <arguments>
mvwgetnstr(welcome, welcomeRow, 45, userInput, MAX_USER_INPUT+256);
strncpy(sendbuffer, userInput, MAX_USER_INPUT + 256);
fflush(stdin);
//gets only the command from the input
userCommand = strtok(userInput, " ");

if (!strcmp(userCommand, "\\join"))
{			
//sends the whole input to the server
//
pthread_mutex_lock(&writeMutex);
write(session->socket_descriptor, sendbuffer, strlen(sendbuffer));	
puts(sendbuffer);
pthread_mutex_unlock(&writeMutex);

read(session->socket_descriptor, &canEnterRoom, sizeof(int));

selectedRoom = strtok(NULL, " ");
if(canEnterRoom == 1)
{
pthread_mutex_lock(&choosingRoomMutex);
choosingRoom = 0;
pthread_mutex_unlock(&choosingRoomMutex);
//drawChat();
pthread_mutex_lock(&inChatMutex);
in_chat = 1;
pthread_mutex_unlock(&inChatMutex);
listen_to_msgs();

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
	if(!strcmp(buffer,"wait")) {}
	}
}
else if (!strcmp(userCommand, "\\quit")) 
{
	//sends the whole input to the server
	pthread_mutex_lock(&writeMutex);
	write(session->socket_descriptor, sendbuffer, strlen(sendbuffer));	
	pthread_mutex_unlock(&writeMutex);
	endwin();			
	close(session->socket_descriptor);
	pthread_mutex_lock(&choosingRoomMutex);
	choosingRoom = 0;
	pthread_mutex_unlock(&choosingRoomMutex);

}
else if (!strcmp(userCommand, "\\create"))
{
	//sends the whole input to the server
	pthread_mutex_lock(&writeMutex);
	write(session->socket_descriptor, sendbuffer, strlen(sendbuffer));	
	pthread_mutex_unlock(&writeMutex);
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
		if(!strcmp(buffer,"wait")) {}
}
else if (!strcmp(userCommand, "\\nick"))
{
	clear_last_line();
	clear_command_input();
	pthread_mutex_lock(&writeMutex);			
	write(session->socket_descriptor, sendbuffer, strlen(sendbuffer));	
	pthread_mutex_unlock(&writeMutex);
}
else {
	clear_last_line();
	clear_command_input();
	pthread_mutex_lock(&scrMutex);	
	mvwprintw(welcome, 22, 2, "Invalid command");
	wrefresh(welcome);			
	pthread_mutex_unlock(&scrMutex);

	//The command is invalid. Wait until the server refreshes the rooms list.
}

}
}	
}
*/


