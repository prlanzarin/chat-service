#ifndef __message__
#define __message__

#define MAX_MESSAGE_SIZE 1024

#define QUIT "\\quit"
#define ROOM_CREATION "\\create"
#define ROOM_DELETION
#define USER_JOIN_ROOM "\\join"
#define USER_LEAVE_ROOM "\\leave"
#define USER_SEND_MESSAGE_TO_ROOM "\\"
#define USER_SEND_PRIVATE_MESSAGE "\\whisper"
#define ROOM_ADD_USER 
#define ROOM_KICK_USER
#define SESSION_CREATE_USER
#define SESSION_USER_LOGIN
#define USER_LOGOFF
#define HELP "\\help"
#define LIST_ROOMS "\\ls"
#define NICK "\\nick"

struct message {
        int op; // operation ID. It'll define how data will be deserialized
        int who; // session ID
        size_t data_size; // bytes of data field
        char *data; // data buffer to be (de)serialized in spite of 'op'
} __attribute__((packed));

typedef struct message MESSAGE; 
/*
enum chat_ops { 
        QUIT,
        ROOM_CREATION, // TODO ASAP
        ROOM_DELETION, // TODO ASAP
        USER_JOIN_ROOM, // TODO ASAP
        USER_LEAVE_ROOM, // TODO ASAP
        USER_SEND_MESSAGE_TO_ROOM, // TODO ASAP
        USER_SEND_PRIVATE_MESSAGE, //TODO  
        ROOM_ADD_USER, //TODO 
        ROOM_KICK_USER, //TODO 
        SESSION_CREATE_USER, //TODO
        SESSION_USER_LOGIN, //TODO
        USER_LOGOFF, //TODO
        HELP,
        LIST_ROOMS,
        NICK
};
*/
unsigned char *char_serialize(unsigned char *buffer, char c);

unsigned char *int_serialize(unsigned char *buffer, int n);

unsigned char *char_deserialize(unsigned char *buffer, char *c);

unsigned char *int_deserialize(unsigned char *buffer, int *n);

MESSAGE *MESSAGE_new(int op, int who, size_t data_size, ...);

unsigned char *MESSAGE_serialize(MESSAGE *msg);

MESSAGE *MESSAGE_deserialize(unsigned char *msg_buf);

void MESSAGE_destroy(MESSAGE *msg);

#endif
