#include "client_list.h"

#include <sys/select.h>
#include <netinet/in.h>

client *client_list = NULL;

client *first_client(){
    return client_list;
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

int add_client( client* new_client){
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
        if (current_client->socket_listen > *max_fd){
            *max_fd = current_client->socket_listen;
        }
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
            return 0;
        }
        current_client = current_client->next;
    }
    return -1;
}