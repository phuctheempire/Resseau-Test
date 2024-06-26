#include "client_list.h"

#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>

static client *client_list = NULL;
static int number_of_client = 0;

int get_number_of_client(){
    return number_of_client;
}
client *first_client(){
    return client_list;
}

int affiche_client(){
    client* current_client = client_list;
    while (current_client != NULL){
        printf("--------------------------------------------\n");
        printf("Client with port %d\n", current_client->port);
        current_client = current_client->next;
    }
    return 0;
}

int port_exist( client* targeted_client, int port){
    client* current_client = client_list;
    while (current_client != NULL){
        if (current_client->port == port && current_client != targeted_client){
            return 1;
        }
        current_client = current_client->next;
    }
    return 0;
}

uint32_t* get_all_ip_port( client* requested_client ){
    uint32_t* ip_port = calloc( 2*(number_of_client - 1 ), sizeof(uint32_t*));
    client* current_client = first_client();
    int i = 0;
    while (current_client != NULL){
        if (current_client != requested_client){
            ip_port[2*i] = current_client->port;
            ip_port[2*i + 1] = current_client->sockaddr_client.sin_addr.s_addr;
            i++;
        }
        current_client = current_client->next;
    }
    return ip_port;
}

client* add_client( int socket_client, int port, struct sockaddr_in sockaddr_client){
    client* new_client = calloc( sizeof(client), 1);
    if (new_client == NULL){
        return NULL;
    }
    new_client->socket_client = socket_client;
    new_client->port = port;
    new_client->sockaddr_client = sockaddr_client;
    new_client->next = NULL;
    append_client(new_client);
    number_of_client += 1;
    return new_client;
}

int append_client( client* new_client){
    if ( new_client == NULL){
        return -1;
    }

    if (client_list == NULL){
        client_list = new_client;
        return 0;
    }
    else{
        client* last = last_client();
        last->next = new_client;
    }
    new_client -> next = NULL;
}

client* last_client(){
    if (client_list == NULL){
        return NULL;
    }
    client* current_client = client_list;
    while (current_client->next != NULL){
        current_client = current_client->next;
    }
    return current_client;
}

void set_max_fd_all_client(fd_set *fd_listen, int *max_fd){
    client* current_client = client_list;
    while( current_client != NULL){
        if (current_client->socket_client > *max_fd){
            *max_fd = current_client->socket_client;
        }
        FD_SET(current_client->socket_client, fd_listen);
        current_client = current_client->next;
    }
}

int remove_client( client* targeted_client){
    if (targeted_client == NULL){
        return -1;
    }
    if ( client_list == targeted_client){
        client_list = NULL;
        free(targeted_client);
        return 0;
    }
    client* current_client = client_list;
    while (current_client->next != NULL){
        if (current_client->next == targeted_client){
            current_client->next = targeted_client->next;
            free(targeted_client);
            close(targeted_client->socket_client);
            return 0;
        }
        current_client = current_client->next;
    }
    number_of_client -= 1;
    return -1;
}