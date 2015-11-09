#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> 
#include "../include/server.h"
#include "../include/room.h"

void *connection_handler(void *);
int free_session_index(SESSION session_list[]);

SERVER *server;
pthread_mutex_t roomListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sessionAssignMutex = PTHREAD_MUTEX_INITIALIZER;
int curr_session = -1;

int main(int argc , char *argv[])
{
	SESSION *s_ptr;
	int i;
	ROOM testrooms[10];	

	if((server = SERVER_new(AF_INET, PORT, NULL)) == NULL) {
		fprintf(stderr, "ERROR: server creation failed.\n");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < 10; i++)
	{
		testrooms[i].name[0] = 'a'+i;
		testrooms[i].name[1] = '\0';
		testrooms[i].online_users = NULL;
		server->rooms =	LIST_push(server->rooms, &testrooms[i]);
	}

	curr_session = free_session_index(server->sessions);	

	// server stays listening on its socket for new client connections
	listen(server->socket_descriptor, BACKLOG_CONNECTIONS);

	// connection capture
	puts("Waiting for incoming connections...");
	s_ptr = &server->sessions[0];
	while(1) {
		if((s_ptr[curr_session].socket_descriptor =
					accept(server->socket_descriptor, 
						(struct sockaddr *) &s_ptr[curr_session].client_socket, 
						(socklen_t*) &SOCKADDR_IN_SIZE)) < 0)
			fprintf(stderr, "ERROR: connection could not be accepted\n");


		if(curr_session != -1) 
			printf("FREE SESSION FOUND: %d\n", curr_session);
		else { 
			//send_message to client of no free sessions
		}
		s_ptr[curr_session].valid = 1;

		if(pthread_create(&s_ptr[curr_session].session_thread, NULL, 
					connection_handler, 
					(void*)&s_ptr[curr_session]) < 0) {
			perror("ERROR: new server thread could not be created");
			exit(EXIT_FAILURE);
		}

		// now join the thread , so that we dont terminate before the thread
		//pthread_join(s_ptr[curr_session].session_thread, NULL);
		puts("Handler assigned");
		curr_session = free_session_index(server->sessions);	
	}
	return 0;
}

/*
 * This will handle connection for each client
 */
void *connection_handler(void *in)
{
	//Get the socket descriptor
	SESSION *session = (SESSION *)in;
	int sock = session->socket_descriptor, inc_size;
	size_t msg_size = 0;
	char buffer[MAX_MESSAGE_SIZE];
	char *message, *ptr;

	//Send some messages to the client
	message = "	Hello! Type \\nick <name> to enter your name:";
	SERVER_send_message(sock, message);
	while(1) {
		memset(buffer, 0, sizeof(buffer));
		inc_size = read(sock, &msg_size, sizeof(int));
		if(inc_size < 0) {
			fprintf(stderr, "ERROR: could not read from client socket\n");
			exit(EXIT_FAILURE);
		}
		// /quit command
		if(inc_size == 0)
			break;

		while(msg_size > 0) {
			inc_size = read(sock, ptr, msg_size);
			if(inc_size < 0) {
				fprintf(stderr, "ERROR: could not read from client socket\n");
				exit(EXIT_FAILURE);
			}
			// /quit command
			if(inc_size == 0)
				break;
			msg_size = msg_size - inc_size;
			ptr = ptr + inc_size;
		}

		if (!strstr(buffer, USER_SEND_MESSAGE_TO_ROOM))
			SERVER_room_broadcast(session, 
					buffer + strlen(USER_SEND_MESSAGE_TO_ROOM));

		else if (!strstr(buffer, "\\leave"))
			SERVER_leave_room(session, buffer + 1);

		else if (!strstr(buffer, "\\join"))
			SERVER_join_room(session, buffer + 1);

		else if (!strstr(buffer, "\\quit")) 
			SERVER_session_disconnect(session);

		else if (!strstr(buffer, "\\create"))
			SERVER_create_room(session, buffer + 1);

		else if (!strstr(buffer, "\\nick"))
			SERVER_new_user(session, buffer + 1);

		else if (!strstr(buffer, "\\ls"))
			SERVER_list(session, buffer + 1);

		else if (!strstr(buffer, "\\help"))
			SERVER_help(session, buffer + 1);

		else if (!strstr(buffer, "\\whisper"))
			SERVER_send_whisper(session, buffer + 1);

		else if (SERVER_invalid_command(buffer))
			SERVER_send_message(sock, "INVALID COMMAND!");
		else 
			SERVER_send_message(sock, "INVALID COMMAND!");
	}

	SERVER_session_disconnect(session);
	pthread_exit(0);
} 

int SERVER_new_user(SESSION *session, char *buffer) {
	char *snd_buf = malloc(sizeof(MAX_MESSAGE_SIZE));
	if(USER_change_name(session->user, buffer) < 0)
		SERVER_send_message(session->socket_descriptor, "NAME TOO LARGE");
	else {
		strncpy(snd_buf, "Your new nick is ", MAX_MESSAGE_SIZE);
		strncat(snd_buf, session->user->name, MAX_MESSAGE_SIZE);
		SERVER_send_message(session->socket_descriptor, snd_buf);
	}	
	free(snd_buf);
	return 1;
}

int SERVER_create_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);
	pthread_mutex_lock(&roomListMutex);
	if (new_room == NULL)
	{
		ROOM *new_room = ROOM_create(buffer, session->user);	
		if(new_room == NULL) {
			SERVER_send_message(session->socket_descriptor, "NAME TOO LARGE");
			return -1;
		}
		server->rooms = LIST_push(server->rooms, new_room);
		pthread_mutex_unlock(&roomListMutex);
		return 1;
	}
	else
	{
		fprintf(stderr, "ERROR: room name already exists");
		SERVER_send_message(session->socket_descriptor, "ROOM ALREADY EXISTS");
		pthread_mutex_unlock(&roomListMutex);
		return -1;
	}
	return 1;
}

int SERVER_join_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);

	if(new_room == NULL) {
		SERVER_send_message(session->socket_descriptor, 
				"THERE IS NO ROOM WITH THAT NAME");
		return -1;
	}

	if(session->user->in_chat) {
		SERVER_send_message(session->socket_descriptor, 
				"ALREADY ON CHAT");
		return -1;
	}

	ROOM_add_user(new_room, session->user);
	char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
	strncpy(snd_buf, session->user->name, MAX_USER_NAME);
	strncat(snd_buf, " has joined the room!", MAX_MESSAGE_SIZE);
	SERVER_room_broadcast(session, snd_buf);
	free(snd_buf);

	return 0;
}

int SERVER_leave_room(SESSION *session, char *buffer) {
	ROOM *new_room = SERVER_get_room_by_name(buffer);

	if(new_room == NULL) {
		SERVER_send_message(session->socket_descriptor, 
				"THERE IS NO ROOM WITH THAT NAME");
		return -1;
	}

	if(!(session->user->in_chat)) {
		SERVER_send_message(session->socket_descriptor, 
				"ALREADY ON CHAT");
		return -1;
	}

	ROOM_kick_user(new_room, session->user);
	char *snd_buf = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
	strncpy(snd_buf, session->user->name, MAX_USER_NAME);
	strncat(snd_buf, " has left the room!", MAX_MESSAGE_SIZE);
	SERVER_room_broadcast(session, snd_buf);
	free(snd_buf);

	return 0;
}

int SERVER_room_broadcast(SESSION *session, char *buffer) {
	SESSION *session_ptr = NULL;
	ROOM *room = session->user->room;
	LIST *users = room->online_users;
	USER *user = (USER *) (users->node);
	pthread_mutex_lock(&roomListMutex);
	while (users)
	{
		user = (USER *) (users->node);
		session_ptr = SERVER_get_session_by_user(user->name);
		SERVER_send_message(session_ptr->socket_descriptor, buffer);
		users = users->next; 
	}
	pthread_mutex_unlock(&roomListMutex);
	return 0;
}

int SERVER_send_whisper(SESSION *session, char *buffer) {
	return 0;
}

int SERVER_help(SESSION *session, char *buffer) {
	return 0;
}

int SERVER_list(SESSION *session, char *buffer) {
	return 0;
}

int SERVER_session_disconnect(SESSION *session) {
	return 0;
}

int SERVER_send_message(int socket, char *buffer) {
	return 0;
}

int SERVER_invalid_command(char *buffer) {
	return 0;
}

/*
 * Returns the index of the first available (disconnected) session
 */
int free_session_index(SESSION session_list[]) {
	int i = 0;
	pthread_mutex_lock(&userListMutex);
	while(session_list[i].valid != 0 && i < MAX_SESSIONS)
		i++;
	if(i == MAX_SESSIONS)
	{
		pthread_mutex_unlock(&userListMutex);
		return -1;
	}
	pthread_mutex_unlock(&userListMutex);
	return i;
}

/* 
 * Instantiates a new SERVER struct (returns a pointer to it, it must be freed
 * later
 */
SERVER *SERVER_new(short family, unsigned short port, USER *admin) {

	SERVER *server = (SERVER *) malloc(sizeof(SERVER));
	/* creates a socket with TCP protocol */
	server->socket_descriptor = socket(family, SOCK_STREAM, 0);
	if (server->socket_descriptor == -1) {
		fprintf(stderr, "ERROR: could not open server socket\n");
		return NULL;
	}
	puts("SERVER SOCKET CREATION OK");

	/* server socket init */
	server->server_socket.sin_family = family;
	server->server_socket.sin_addr.s_addr = INADDR_ANY;
	// htons is for endian compatibility
	server->server_socket.sin_port = htons(port); 

	/* server socket binding */
	if(bind(server->socket_descriptor,
				(struct sockaddr *)(&server->server_socket),
				sizeof(server->server_socket)) < 0) {
		fprintf(stderr, "ERROR: could not bind server socket\n");	
		return NULL;
	}
	puts("SERVER SOCKET BINDING OK");

	server->server_admin = admin;
	int i;
	for(i = 0; i < MAX_SESSIONS; i++)
		SESSION_set(&server->sessions[i], AF_INET, PORT, SERVER_HOSTNAME,
				PORT, NULL); 

	server->rooms = NULL; //LIST_create(lobby);

	return server;
}

/*
   USER *SERVER_get_user_by_name(char *userName)
   {
   LIST *users = server->users;
   USER *check;
   if (userName == NULL)
   {
   fprintf(stderr,"ERROR: Null pointer to user name!");
   return NULL;
   }

   pthread_mutex_lock(&userListMutex);
   while (users)
   {
   check = (USER *) (users->node);
   if (!strcmp (userName, check->name))
   {	
   pthread_mutex_unlock(&userListMutex);
   return check;
   }

   users = users->next;
   }
   pthread_mutex_unlock(&userListMutex);
   return NULL;
   }
   */
ROOM *SERVER_get_room_by_name(char *roomName)
{	
	LIST *rooms = server->rooms;
	ROOM *check;
	if (roomName == NULL)
	{
		fprintf(stderr,"ERROR: Null pointer to user name!");
		return NULL;
	}

	pthread_mutex_lock(&roomListMutex);
	while (rooms)
	{
		check = (ROOM *) (rooms->node);
		if (!strcmp (roomName, check->name))
		{	
			pthread_mutex_unlock(&roomListMutex);
			return check;
		}

		rooms = rooms->next;

	}
	pthread_mutex_unlock(&roomListMutex);
	return NULL;
}

SESSION *SERVER_get_session_by_user(char *userName)
{
	int i;
	SESSION *check;
	pthread_mutex_lock(&userListMutex);	
	for (i = 0; i < MAX_SESSIONS; i++)
	{
		check = &(server->sessions[i]);

		if(!strncmp(check->user->name, userName, MAX_USER_NAME)) {
			pthread_mutex_unlock(&userListMutex);
			return check;
		}

	}
	pthread_mutex_unlock(&userListMutex);
	return NULL;
}


/*
   void SERVER_show_rooms(int socket)
   {
   int ack;
   char waitMessage[5] = "wait";
   char finishMessage[7] = "finish";
   int waitAck;
   int finishAck;
   LIST *rooms = server->rooms;
   ROOM *room;

   fprintf(stderr, "Sending available rooms to the client %d\n",socket);

//before showing the rooms, server must inform the listen thread in the client
write(socket, waitMessage, strlen(waitMessage));
read(socket, &waitAck, sizeof(int));

pthread_mutex_lock(&roomListMutex);
while (rooms)
{
room = (ROOM *) (rooms->node);
write(socket, room->name, strnlen(room->name, MAX_ROOM_NAME));
read(socket, &ack, sizeof(int));	
rooms = rooms->next;		
}
pthread_mutex_unlock(&roomListMutex);

write(socket, finishMessage, strlen(finishMessage));
read(socket, &finishAck, sizeof(int));
}

int SERVER_checkRoomExists (char *roomName)
{
LIST *rooms = server->rooms;
ROOM *room;
pthread_mutex_lock(&roomListMutex);
while (rooms)
{
room = (ROOM *) (rooms->node);
//If there is a room with the speficied name, return true
if (!strcmp(room->name, roomName))
return 1;
rooms = rooms->next;
}
pthread_mutex_unlock(&roomListMutex);
return 0;
}

void send_message(ROOM *room, char *message) 
{
LIST *users = room->online_users;
SESSION *session;
USER *user = (USER *) (users->node);
pthread_mutex_lock(&roomListMutex);
while (users)
{
user = (USER *) (users->node);
session = SERVER_get_session_by_user(user->name);
write(session->socket_descriptor, message, strlen(message));
users = users->next; 
}
pthread_mutex_unlock(&roomListMutex);
}

int SERVER_change_user_name(char *user_name, char *new_name) {
USER *user = SERVER_get_user_by_name(user_name);	
if(user == NULL)
return -1;	

strncpy(user->name, new_name, MAX_USER_NAME);
puts(user->name);
return 0;
}

void SERVER_process_user_cmd (int socket, char *userName)
{
	char *userInput = malloc(sizeof(char)*(MAX_USER_INPUT + 256));
	char *selectedRoom = malloc(sizeof(char)*MAX_ROOM_NAME);
	char *msg = malloc(sizeof(char)*256);
	char *roomName = malloc(sizeof(char)*MAX_ROOM_NAME);
	char *newUserName = malloc(sizeof(char)*MAX_USER_NAME);
	char *hasJoinedMessage = malloc(sizeof(char)*(MAX_USER_NAME + 22));
	char *who_what = malloc(sizeof(char) * (MAX_USER_NAME + 256 + 9));
	int canCreateRoom, canEnterRoom, QUIT = 0, IN_ROOM = 0;
	ROOM *targetRoom = NULL;
	USER *targetUser;
	LIST *templist;


	targetUser = SERVER_get_user_by_name(userName);

	while(!QUIT) {
		//reads the user input


		memset(userInput, 0, (sizeof(char)*(MAX_USER_INPUT + 256)));
		memset(selectedRoom, 0, (sizeof(char)*MAX_ROOM_NAME));
		//memset(msg, 0, sizeof(char) *256);
		memset(roomName, 0, sizeof(char) *MAX_ROOM_NAME);
		memset(newUserName, 0, sizeof(char) *MAX_USER_NAME);
		memset(hasJoinedMessage, 0, sizeof(char)*(MAX_USER_NAME+22));
		memset(who_what, 0, sizeof(char)*(MAX_USER_NAME+256+9));


		fprintf(stderr,"Waiting for client %s's commands...\n",userName);
		read(socket, userInput, MAX_USER_INPUT);
		fprintf(stderr, "userInput: %s\n", userInput);
		strtok(userInput, " ");
		fprintf(stderr, "userInput depois do strtok: %s\n", userInput);

		fprintf(stderr, "Server: userName antes de verificar o userInput: %s\n", userName);

		if (!strcmp(userInput, "\\join"))
		{
			selectedRoom = strtok(NULL, " ");

			//waits for the client to choose the room
			fprintf(stderr, "User %s wants to join room %s.\n", userName, selectedRoom);
			targetRoom = SERVER_get_room_by_name(selectedRoom);
			if (targetRoom == NULL)	{
				canEnterRoom = 0;
				write(socket, &canEnterRoom, sizeof(int));
				SERVER_show_rooms(socket);
			}
			else {
				canEnterRoom = 1;			
				write(socket, &canEnterRoom, sizeof(int));
				//Adds the user to the specified room
				ROOM_add_user(targetRoom, targetUser);

				fprintf(stderr, "Server: userName antes da strcpy: %s\n", userName);
				//warns the other clients in the room that a new client has joined
				strcpy(hasJoinedMessage, userName);
				strcat(hasJoinedMessage, " has joined the room.");

				send_message(targetRoom, hasJoinedMessage); 
				IN_ROOM = 1;
			}
			//fprintf(stderr, "Server: listenToMsgs will receive userName %s\n", userName);
			//server listens to msgs in the chat
			//listenToMsgs(userName, roomIndex, socket);

		}
		else if (!strcmp(userInput, "\\quit"))
		{
			templist = LIST_create((void *)targetUser);
			QUIT = 1;
			ROOM_kick_user(targetRoom, targetUser);
			LIST_remove(server->users, templist);
			close(socket);

		}
		else if (!strcmp(userInput, "\\send"))
		{
			strcpy(who_what, userName);
			strcat(who_what, " says:");

			while((msg = strtok(NULL, " ")) != NULL) {
				strcat(who_what, msg);
				strcat(who_what, " ");
			}

			//waits for the client to choose the room
			//warns the other clients in the room that a new client has joined
			send_message(targetRoom, who_what); 

		}
		else if (!strcmp(userInput, "\\leave"))
		{
			if(targetRoom == NULL || targetUser == NULL) {
				IN_ROOM = 1;
				write(socket, &IN_ROOM, sizeof(int));
			}
			else {	
				IN_ROOM = 0;
				ROOM_kick_user(targetRoom, targetUser);
				write(socket, &IN_ROOM, sizeof(int));
				SERVER_show_rooms(socket);
			}
		}
		else if (!strcmp(userInput, "\\nick"))
		{
			newUserName = strtok(NULL, " ");
			fprintf(stderr, "Server: Client wants to change his name to %s\n", newUserName);
			strncpy(userName, newUserName, MAX_USER_NAME);
			SERVER_change_user_name(userName, newUserName);
			fprintf(stderr, "Server: nome do cliente apos changeuserName: %s\n", newUserName);
			SERVER_show_rooms(socket);
			fflush(stdin);
			//checkUserInputFromOutside(socket, newuserName);
		}
		else if (!strcmp(userInput, "\\create"))
		{
			roomName = strtok(NULL, " ");
			fprintf(stderr, "User %s wants to create room %s\n", userName, roomName);

			if(SERVER_checkRoomExists(roomName)) {
				canCreateRoom = 0;
				write(socket, &canCreateRoom, sizeof(int));			
			}
			else if (roomName == NULL)
			{
				canCreateRoom = 0;
				write(socket, &canCreateRoom, sizeof(int));			
			}
			else {
				canCreateRoom = 1;
				write(socket, &canCreateRoom, sizeof(int));
				SERVER_create_room(SERVER_get_user_by_name(userName), roomName);			
			}
			SERVER_show_rooms(socket);
		}
		else {
			SERVER_show_rooms(socket);
		}	

	}
}
*/
