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
	char *message;
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
		server->rooms =	LIST_push(server->rooms, &testrooms[i]);
	}

	curr_session = free_session_index(server->sessions);	

	// server stays listening on its socket for new client connections
	listen(server->socket_descriptor, BACKLOG_CONNECTIONS);

	// connection capture
	puts("Waiting for incoming connections...");
	s_ptr = &server->sessions[0];
	while((s_ptr[curr_session].socket_descriptor =
				accept(server->socket_descriptor, 
					(struct sockaddr *) &s_ptr[curr_session].client_socket,
					(socklen_t*) &SOCKADDR_IN_SIZE))) {

		if(curr_session != -1) 
			printf("FREE SESSION FOUND: %d\n", curr_session);
		else 
			break;
		s_ptr[curr_session].valid = 1;
		// reply to the client
		message = "	Hello Client, you'll be handled properly\n"; 
		write(s_ptr[curr_session].socket_descriptor, 
				message, strlen(message));

		if(pthread_create(&s_ptr[curr_session].session_thread, NULL, 
					connection_handler, 
					(void*)&s_ptr[curr_session]) < 0) {
			perror("ERROR: new server thread could not be created");
			exit(EXIT_FAILURE);
		}

		// now join the thread , so that we dont terminate before the thread
		pthread_join(s_ptr[curr_session].session_thread, NULL);
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
	int sock = session->socket_descriptor;
	char buffer[256];
	char userName[MAX_USER_NAME];

	char *message;
	USER *newUser;
	

	//Send some messages to the client
	message = "	Greetings! I am your connection handler\n";
	write(sock, message, strlen(message));

	message = "	Its my duty to communicate with you\n";
	write(sock, message, strlen(message));

	message = "	Type your nickname:";
	write(sock, message, strlen(message));

	//Receive the chosen nickname
	if(read(sock, buffer, sizeof(buffer)) > 0)
		printf ("	The chosen nickname is %s\n",buffer);

        //Default password, for now.
	newUser = USER_create(buffer, 0);
	//Adds the new user to the list
	SERVER_new_user(sock, newUser); 
		
	//Send the available rooms to the client
	SERVER_show_rooms(sock);

	strncpy(userName,buffer,sizeof(userName));
	SERVER_process_user_cmd(sock, userName);

	//Free the socket pointer
	pthread_mutex_lock(&sessionAssignMutex);
	printf("CLOSING SESSION %d\n", sock);
	session->valid = 0;
	pthread_mutex_unlock(&sessionAssignMutex);
	return 0;
} 
/*
 *
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

	ROOM *lobby = ROOM_create("Lobby", admin);	
	server->rooms = LIST_create(lobby);
	server->users = LIST_create(admin);

	return server;
}

int SERVER_new_user(int socket_desc, USER *newUser) {
	int i;	
	SESSION *check;
	if (newUser == NULL)
	{
		fprintf(stderr,"ERROR: Null pointer to new user!");
		return -1;
	}
	pthread_mutex_lock(&userListMutex);
	server->users = LIST_push(server->users, newUser);
	for (i = 0; i < MAX_SESSIONS; i++)
	{
		check = &(server->sessions[i]);
		//Associate the new user with its session
		if(check->socket_descriptor == socket_desc)
		{
			check->user = newUser; 
			break;
		}
	}
	pthread_mutex_unlock(&userListMutex);
	return 1;
}

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
	}
	pthread_mutex_unlock(&userListMutex);
	return NULL;
}

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
		if(check->valid) {
			if(!strncmp(check->user->name, userName, MAX_USER_NAME))
			{
				pthread_mutex_unlock(&userListMutex);
				return check;
			}
		}
		else
			break;
	}
	pthread_mutex_unlock(&userListMutex);
	return NULL;
}

int SERVER_create_room(USER *user, char *room_name)
{
	ROOM *newRoom = ROOM_create(room_name, user);	
	pthread_mutex_lock(&roomListMutex);
	if (newRoom != NULL)
	{
		server->rooms = LIST_push(server->rooms, newRoom);
		pthread_mutex_unlock(&roomListMutex);
		return 1;
	}
	else
	{
		fprintf(stderr,"ERROR: Couldn't create new room!");
		pthread_mutex_unlock(&roomListMutex);
		return -1;
	}
}


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
	fprintf(stderr, "Sent waitMessage: %s\n", waitMessage);
	read(socket, &waitAck, sizeof(int));
	fprintf(stderr, "Received waitAck %d\n", waitAck);

	pthread_mutex_lock(&roomListMutex);
	while (rooms)
	{
		room = (ROOM *) (rooms->node);
		write(socket, room->name, strnlen(room->name, MAX_ROOM_NAME));
		read(socket, &ack, sizeof(int));	
		fprintf(stderr, "Received roomAck %d\n",ack);
		rooms = rooms->next;		
	}
	pthread_mutex_unlock(&roomListMutex);

	fprintf(stderr, "Finished sending available rooms.\n");
	fprintf(stderr, "Sending finishMessage: %s\n", finishMessage);
	write(socket, finishMessage, strlen(finishMessage));
	read(socket, &finishAck, sizeof(int));
	fprintf(stderr, "Received finishAck %d\n", finishAck);
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
	while (users != NULL)
	{
		session = SERVER_get_session_by_user(user->name);
		write(session->socket_descriptor, message, strlen(message));
		users = users->next; 
		user = (USER *) (users->node);
	}
	pthread_mutex_unlock(&roomListMutex);
}

void SERVER_process_user_cmd (int socket, char *userName)
{
	
	char *userInput = malloc(sizeof(char)*MAX_USER_INPUT);
	char *selectedRoom = malloc(sizeof(char)*MAX_ROOM_NAME);
	char *roomName = malloc(sizeof(char)*MAX_ROOM_NAME);
	int canCreateRoom;
	int canEnterRoom;
	ROOM *targetRoom;
	USER *targetUser;

	memset(userInput, 0, MAX_USER_INPUT);

	//reads the user input
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
		targetUser = SERVER_get_user_by_name(userName);
		if (targetRoom == NULL)		
			canEnterRoom = 0;
		else
			canEnterRoom = 1;			
		write(socket, &canEnterRoom, sizeof(int));
		
		//Adds the user to the specified room
		ROOM_add_user(targetRoom, targetUser);
		
		send_message(targetRoom, "olá");
		fprintf(stderr, "depois do send:\n");
		/*fprintf(stderr, "Server: userName antes da strcpy: %s\n", userName);
		//warns the other clients in the room that a new client has joined
		strcpy(hasJoinedMessage, userName);
		strcat(hasJoinedMessage, " has joined the room.");
		writeToRoom(hasJoinedMessage, roomIndex, socket);

		fprintf(stderr, "Server: listenToMsgs will receive userName %s\n", userName);
		//server listens to msgs in the chat
		listenToMsgs(userName, roomIndex, socket);*/
	}
	else if (!strcmp(userInput, "\\quit"))
	{
		close(socket);
			
	}
	else if (!strcmp(userInput, "\\nick"))
	{
		/*newuserName = strtok(NULL, " ");
		fprintf(stderr, "Server: Client wants to change his name to %s\n", newuserName);
		changeuserName(userName, newuserName);
		fprintf(stderr, "Server: nome do cliente apos changeuserName: %s\n", newuserName);
		showRooms(socket);
		checkUserInputFromOutside(socket, newuserName);*/
	}
	else if (!strcmp(userInput, "\\create"))
	{
		roomName = strtok(NULL, " ");
		fprintf(stderr, "User %s wants to create room %s\n", userName, roomName);

		if(SERVER_checkRoomExists(roomName)) {
			fprintf(stderr, "ERROR: Room with name %s already exists!\n", roomName);
			canCreateRoom = 0;
			write(socket, &canCreateRoom, sizeof(int));			
		}
		else if (roomName == NULL)
		{
			fprintf(stderr, "ERROR: Attempt to create nameless room!\n");
			canCreateRoom = 0;
			write(socket, &canCreateRoom, sizeof(int));			
		}
		else {
			fprintf(stderr, "Room with name %s will be created.\n", roomName);
			canCreateRoom = 1;
			write(socket, &canCreateRoom, sizeof(int));
			SERVER_create_room(SERVER_get_user_by_name(userName), roomName);			
		}
		SERVER_show_rooms(socket);
		SERVER_process_user_cmd(socket, userName);
	}
	else {
		SERVER_show_rooms(socket);
		SERVER_process_user_cmd(socket, userName);
	}	

}
