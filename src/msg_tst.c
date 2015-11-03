#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/message.h"

int main(int argc, char *argv[]) {
	char *buffer = (char *)malloc(sizeof(char) * 8); 
	char *buffert = (char *)malloc(sizeof(char) * 8); 
	size_t data_size = sizeof(char) * 8;
	int op = 1; int who = 2;
	strncpy(buffer, "MENSAGEM", data_size);
//	strncpy(buffert, "AAAAAAAA", data_size);

	MESSAGE *msg;
	msg = MESSAGE_new(op, who, data_size, buffer);
	printf("NEW: %d %d %u\n", msg->op, msg->who, msg->data_size);
	puts(msg->data);
	unsigned char uc;
	char c;
	char_serialize(&uc, 'c');
	char_deserialize(&uc, &c);
	printf("%u\n", c);
	printf("%u\n", uc);
	unsigned char *iuc = (unsigned char *)malloc(sizeof(unsigned char) * 
			sizeof(int));
	int_serialize(iuc, 182);
	int *n = (int *) malloc(sizeof(int));;
	int_deserialize(iuc, n);
	printf("N: %d\n", *n);

/*	
	unsigned char *ser_msg = MESSAGE_serialize(msg);
	puts(ser_msg);
	MESSAGE *rebuilt_msg = MESSAGE_deserialize(ser_msg);	
	printf("REBUILT: %d %d %u\n", rebuilt_msg->op, rebuilt_msg->who, 
			rebuilt_msg->data_size);
	puts(rebuilt_msg->data);
*/
	return 0; 
}
