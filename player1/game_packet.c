#include "game_packet.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>

static int player_port = 0;


game_packet *create_game_packet(){
    game_packet *new_packet = calloc(sizeof(game_packet), 1);
    if (new_packet == NULL){
        return NULL;
    }
    return new_packet;
}
void init_game_packet( game_packet* packet, uint8_t type, uint32_t size ){
    packet->type = type;
    packet->size = size;
    packet->data = calloc(size, 1);
}