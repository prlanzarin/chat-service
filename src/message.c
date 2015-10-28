#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/message.h"

MESSAGE *MESSAGE_new(int op, int who, char *data, size_t data_size) {
	MESSAGE *new_msg = (MESSAGE *) malloc(sizeof(MESSAGE));
	new_msg->data = (char *) malloc(sizeof(data_size+1));
	memcpy(new_msg->data, data, data_size);
	free(data);
	new_msg->op = op;
	new_msg->who = who;
	new_msg->data_size = data_size;
	return new_msg;
}

char *MESSAGE_serialize(MESSAGE *msg) {
	return NULL;
}

MESSAGE *MESSAGE_deserialize(char *msg) {
	return NULL;
}

void MESSAGE_destroy(MESSAGE *msg) {
	free(msg->data);
	free(msg);	
	return; 
}