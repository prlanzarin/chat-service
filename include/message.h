#ifndef __message__
#define __message__

typedef struct message {
        int op; // operation ID
        int who; // session ID
        char *data; // data buffer to be (de)serialized in spite of 'op'
} MESSAGE;


#endif
