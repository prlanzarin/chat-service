#ifndef __client__
#define __client__

#define PORT 4000

typedef struct user USER; // The compiler must be aware of the USER structure

size_t CLIENT_STRUCT_SIZE = 0;

typedef struct client {
        int socket_descriptor;
        struct sockaddr_in client_socket;
        pthread_t session_thread;
        USER *user;
} CLIENT;

CLIENT *CLIENT_session_create(USER **user);
#endif
