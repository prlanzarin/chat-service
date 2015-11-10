#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
	return (buffer + sizeof(char));
}

/* serializes an int into an unsigned buffer (there's no boundary checking
 * in the buffer, be careful)
 */
unsigned char *int_serialize(unsigned char *buffer, int n) {
	buffer[0] = n >> 8;
	buffer[1] = n;
	return (buffer + (sizeof(int)/2));
}

/* deserializes an unsigned buffer byte into an char (there's no boundary checking
 * in the buffer, be careful)
 */
unsigned char *char_deserialize(unsigned char *buffer, char *c) {
	*c = buffer[0];
	return (buffer + sizeof(char));
}

/* deserializes an unsigned buffer into an int (there's no boundary checking
 * in the buffer, be careful)
 */
unsigned char *int_deserialize(unsigned char *buffer, int *n) {
	memcpy(n, buffer, sizeof(int));
	printf("DES %d\n", *n);
	return (buffer + sizeof(int));
}

/* module subroutines for MESSAGE management and (de)serialization */
MESSAGE *MESSAGE_new(int op, int who, size_t data_size, ...) {
	//size_t data_size;
	va_list args;
	MESSAGE *new_msg = (MESSAGE *) malloc(sizeof(MESSAGE));
	new_msg->data_size = data_size;
	new_msg->op = op;
	new_msg->who = who;
	new_msg->data = (char *) malloc(sizeof(new_msg->data_size+1));
	va_start(args, data_size);
	vsnprintf(new_msg->data, new_msg->data_size+1, "%s", args);
	va_end(args);
	return new_msg;
}

unsigned char *MESSAGE_serialize(MESSAGE *msg) {
	int i = 0;
	unsigned char *buffer = (unsigned char *) malloc(
			sizeof(unsigned char) * msg->data_size);
	unsigned char *buffer_start = &buffer[0];
	buffer = int_serialize(buffer, msg->op);
	buffer = int_serialize(buffer, msg->who);
	buffer = int_serialize(buffer, msg->data_size);
	for(i = 0; i < msg->data_size; i++) 
		buffer = char_serialize(buffer, msg->data[i]);	

	return buffer_start;
}

MESSAGE *MESSAGE_deserialize(unsigned char *msg) {
	int op, who, i;
	size_t data_size;
	char *buffer; 

	msg = int_deserialize(msg, &op);
	msg = int_deserialize(msg, &who);
	msg = int_deserialize(msg, (int *) &data_size);
	buffer = (char *)malloc(sizeof(data_size));
	for(i = 0; i < data_size; i++)
		msg = char_deserialize(msg, &buffer[i]); 
	
	MESSAGE *new_msg = MESSAGE_new(op, who, data_size, buffer); 
	free(buffer);
	return new_msg;
}

void MESSAGE_destroy(MESSAGE *msg) {
	free(msg->data);
	free(msg);	
	return; 
}


