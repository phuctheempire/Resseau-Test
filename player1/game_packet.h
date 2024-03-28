#ifndef GAME_PACKET
#define GAME_PACKET

#include <stdint.h>
#include "message_type.h"

struct game_packet {
    uint8_t type;
    uint16_t port;
    uint32_t size;
    char* data;
};

typedef struct game_packet game_packet;
extern game_packet* create_game_packet();
extern void init_game_packet(game_packet* packet, uint8_t type, uint32_t size );
extern int has_data( const game_packet* packet);
extern int is_valid_packet( const game_packet* packet);
extern int send_nodata_msg( const uint8_t type, int socket);
extern int send_game_packet( game_packet* packet, int socket);
extern int receive_packet(game_packet* packet, int socket);
extern void flush_socket( int socket);
#endif