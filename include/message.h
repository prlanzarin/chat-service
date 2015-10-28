#ifndef __message__
#define __message__

typedef struct message {
        int op; // operation ID. It'll define how data will be deserialized
        int who; // session ID
        size_t data_size; // bytes of data field
        char *data; // data buffer to be (de)serialized in spite of 'op'
} MESSAGE;

MESSAGE *MESSAGE_new(int op, int who, char *data, size_t data_size);

char *MESSAGE_serialize(MESSAGE *msg);

MESSAGE *MESSAGE_deserialize(char *msg);

void MESSAGE_destroy(MESSAGE *msg);


#endif
