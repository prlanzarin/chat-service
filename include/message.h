#ifndef __message__
#define __message__

struct message {
        int op; // operation ID. It'll define how data will be deserialized
        int who; // session ID
        size_t data_size; // bytes of data field
        char *data; // data buffer to be (de)serialized in spite of 'op'
} __attribute__((packed));

typedef struct message MESSAGE; 

enum chat_ops { 
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
        USER_LOGOFF //TODO
};


MESSAGE *MESSAGE_new(int op, int who, char *data, size_t data_size);

unsigned char *MESSAGE_serialize(MESSAGE *msg);

MESSAGE *MESSAGE_deserialize(unsigned char *msg);

void MESSAGE_destroy(MESSAGE *msg);

#endif
