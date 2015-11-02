#ifndef __client__
#define __client__

#include "../include/message.h"

void *read_message();
void drawWelcome();
void drawChat();
void chooseRoom();
void getAvailableRooms();
void clear_command_input();
void clear_last_line();

#endif
