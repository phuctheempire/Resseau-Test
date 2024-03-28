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


#include "game_packet.h"
#include "client_list.h"



// #define PORT 8000
// #define IP "127.0.0.1"
#define buffer_size 1024
typedef uint32_t game_ip;


extern int connect_to_game();
extern int gameListener(int listen_sock);
extern client* accept_new_client(int listen_sock);





#endif