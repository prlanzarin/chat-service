#ifndef __session__
#define __session__

#define PORT 4000
#define SERVER_HOSTNAME "localhost"

#include <netdb.h>
#include "usr.h"

#define SESSION_STRUCT_SIZE 0

typedef struct session {
        int socket_descriptor;
        struct sockaddr_in client_socket;
        pthread_t session_thread;
        int valid;
        USER *user;
} SESSION;

SESSION *SESSION_new(short family, unsigned short port, USER *user);

int SESSION_set(SESSION *session, short family, unsigned short port, USER *user); 

int SESSION_connect(SESSION *session, char *hostname,
                unsigned short server_port, struct sockaddr_in *server_socket, 
                struct hostent *s_host);
#endif
