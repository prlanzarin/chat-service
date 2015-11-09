#ifndef __client__
#define __client__

#include "../include/message.h"

#define	MAX_USER_COMMAND 10
#define MAX_USER_COMMAND_ARG 20


void *read_message();
void *send_message();
void chooseRoom();
void getAvailableRooms();
void clear_command_input();
void clear_last_line();
void listen_to_msgs();

#endif
