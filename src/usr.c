#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/usr.h"

int USER_create(char *name, int password) {
	return -1;
}

int USER_login(char *name, int password) {
	return -1;
}

int USER_logoff(USER *user); {
	return -1;
}

//FUNDAMENTAL
int USER_join_room(USER *user, ROOM *room) {
	return -1;
}

//FUNDAMENTAL
int USER_leave_room(USER *user, ROOM *room) {
	return -1;
}

int USER_send_message_to_room(USER *user, MESSAGE *msg, ROOM *room) {
	return -1;
}

int USER_send_private_message(USER *from, USER *to, MESSAGE *message) {
	return -1;
}
