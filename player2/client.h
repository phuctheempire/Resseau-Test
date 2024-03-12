#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8888
#define IP "127.0.0.1"
#define buffer_size 1024
typedef uint32_t game_ip;

struct client
{
    int socket;
    uint16_t id;
    struct sockaddr_in sockAddresse;
    struct client *next;
};

typedef struct client client;


struct packet
{
    uint32_t size;
    char data[buffer_size];
};

extern int connect_to_game(game_ip ip, int isNewPlayer);
extern int gameListener(int listen_sock);
extern client* accept_new_client(int listen_sock);





#endif