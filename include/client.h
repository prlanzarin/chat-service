#ifndef __client__
#define __client__

#define PORT 4000
#define SERVER_HOSTNAME "localhost"

#include <sys/types.h>
#include <sys/socket.h>
#include "usr.h"

size_t CLIENT_STRUCT_SIZE = 0;

typedef struct client {
        int socket_descriptor;
        struct sockaddr_in client_socket;
        pthread_t session_thread;
        USER *user;
} CLIENT;

int CLIENT_new_session(CLIENT *session, short family, unsigned short port, 
                char *hostname, USER *user);
#endif
