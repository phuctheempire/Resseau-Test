#ifndef CLIENT_LIST
#define CLIENT_LIST

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>

#include "game_packet.h"


struct client
{
    int socket_listen;
    int socket_send;
    int port;
    struct sockaddr_in sockaddr_listen;
    struct sockaddr_in sockaddr_send;
    struct client *next;
};

typedef struct client client;  
extern client* first_client();
extern int add_client( client* new_client);
extern int port_exist( client* targeted_client, int port);
extern client* last_client();


#endif