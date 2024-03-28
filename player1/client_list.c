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
        if (current_client->port == port){
            return 1;
        }
        current_client = current_client->next;
    }
    return 0;
}