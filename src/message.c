#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/message.h"

/* LISTED CLIENT OPERATIONS:
 * ROOM_CREATION (any user) // TODO URGENT
 * ROOM_DELETION (room admin) // TODO URGENT
 * USER_JOIN_ROOM (any user) // TODO URGENT
 * USER_LEAVE_ROOM (any room participant) // TODO URGENT
 * USER_SEND_MESSAGE_TO_ROOM (any room participant -> room) // TODO URGENT
 * USER_SEND_PRIVATE_MESSAGE (any user -> any user) // TODO URGENT

 * ROOM_ADD_USER (any room participant) // TODO
 * ROOM_KICK_USER (admin) // TODO
 * SESSION_CREATE_USER (any session) // TODO
 * SESSION_USER_LOGIN (any session) // TODO
 * USER_LOGOFF (any user) // TODO
 */

/* non-module subroutines for basic types (de)serialization */

/* serializes a char into an unsigned buffer (there's no boundary checking
 * in the buffer, be careful)
 */
unsigned char *char_serialize(unsigned char *buffer, char c) {
	buffer[0] = c;
	buffer += 1;
	return buffer; 
}

/* serializes an int into an unsigned buffer (there's no boundary checking
 * in the buffer, be careful)
 */
unsigned char *int_serialize(unsigned char *buffer, int n) {
	buffer[0] = n >> 24;
	buffer[1] = n >> 16;
	buffer[2] = n >> 8;
	buffer[3] = n;
	buffer += 4;
	return buffer;
}

/* deserializes an unsigner buffer into an char (there's no boundary checking
 * in the buffer, be careful)
 */
unsigned char *char_deserialize(char *buffer, char *c) {
	return buffer;
}

/* deserializes an unsigner buffer into an int (there's no boundary checking
 * in the buffer, be careful)
 */
unsigned char int_deserialize(unsigned char *buffer, int *n) {
	return buffer;
}

/* module subroutines for MESSAGE management and (de)serialization */
MESSAGE *MESSAGE_new(int op, int who, char *data, size_t data_size) {
	switch(op) {
		case ROOM_CREATION: // TODO ASAP
			break;
		case ROOM_DELETION:// TODO ASAP
			break;
		case USER_JOIN_ROOM: // TODO ASAP
			break;
		case USER_LEAVE_ROOM: // TODO ASAP
			break;
		case USER_SEND_MESSAGE_TO_ROOM: // TODO ASAP
			break;
		case USER_SEND_PRIVATE_MESSAGE: //TODO  
			break;
		case ROOM_ADD_USER: //TODO 
			break;
		case ROOM_KICK_USER: //TODO 
			break;
		case SESSION_CREATE_USER:
			break;	//TODO
		case SESSION_USER_LOGIN:
			break;	//TODO
		case USER_LOGOFF:
			break;//TODO
		default: 
			return NULL;
	}

	MESSAGE *new_msg = (MESSAGE *) malloc(sizeof(MESSAGE));
	new_msg->data = (char *) malloc(sizeof(data_size+1));
	memcpy(new_msg->data, data, data_size);
	free(data);
	new_msg->op = op;
	new_msg->who = who;
	new_msg->data_size = data_size;
	return new_msg;
}

unsigned char *MESSAGE_serialize(MESSAGE *msg) {
	int i = 0;
	unsigned char *buffer = (unsigned char *) malloc(
			sizeof(unsigned char) * msg->data_size);
	buffer = int_serialize(buffer, msg->op);
	buffer = int_serialize(buffer, msg->who);
	for(i = 0; i < msg->data_size; i++) {
		buffer = char_serialize(buffer, msg->data[i]);	
	}
	return buffer;
}

MESSAGE *MESSAGE_deserialize(unsigned char *msg) {
	int op = msg[0];
	switch(op) {
		case ROOM_CREATION: // TODO ASAP
			break;
		case ROOM_DELETION:// TODO ASAP
			break;
		case USER_JOIN_ROOM: // TODO ASAP
			break;
		case USER_LEAVE_ROOM: // TODO ASAP
			break;
		case USER_SEND_MESSAGE_TO_ROOM: // TODO ASAP
			break;
		case USER_SEND_PRIVATE_MESSAGE: //TODO  
			break;
		case ROOM_ADD_USER: //TODO 
			break;
		case ROOM_KICK_USER: //TODO 
			break;
		case SESSION_CREATE_USER:
			break;	//TODO
		case SESSION_USER_LOGIN:
			break;	//TODO
		case USER_LOGOFF:
			break;//TODO
		default: 
			return NULL;
	}
	return NULL;
}

void MESSAGE_destroy(MESSAGE *msg) {
	free(msg->data);
	free(msg);	
	return; 
}


