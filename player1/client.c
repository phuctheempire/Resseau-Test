#include "client.h"

static int main_port;
static char main_ip [1024];
static int new_player = 1;
static int main_listen_sock;

client* accept_new_client( int listen_sock){
    client *new_client = calloc(1, sizeof(client));
    printf("Waiting for new player\n");
    int size = sizeof(new_client->sockaddr_listen);
    if ( (new_client->socket_listen = accept(listen_sock, (struct sockaddr *)&new_client->sockaddr_listen, &size)) <= 0){
        perror("accept");
        return NULL;
    }
    printf("New player connected with socket %d \n", new_client->socket_listen);
    if( add_client(new_client) == -1){
        perror("add_client");
        return NULL;
    }

    if (send_nodata_msg( MSG_CONNECT_START, new_client -> socket_listen) == -1){
        perror("send");
        return NULL;
    }
    printf("Sent welcome message\n");
    printf("Accepted client\n");
    printf("Connecting back to client\n"); 
    // connect_to_game();
    return new_client;
}

int connect_to_game( game_ip ip, int port,  int isNewPlayer){
    struct sockaddr_in sockAddresse ={0};
    game_ip connect_ip = ip;
    sockAddresse.sin_family = AF_INET;
    sockAddresse.sin_port = htons(port);
    sockAddresse.sin_addr.s_addr = connect_ip;

    int newSocket = socket(AF_INET, SOCK_STREAM, 6);

    if ( newSocket == -1){
        perror("socket");
        close(newSocket);
        return 1;
    }
    printf("New socket: %d\n", newSocket);
    if (connect (newSocket, (struct sockaddr *)&sockAddresse, sizeof(sockAddresse) )== -1){
        perror("connect");
        close(newSocket);
        return 1;
    }
    printf("Connected to server\n");
    game_packet *connect_packet = create_game_packet();
    if (connect_packet == NULL){
        return -1;
    }
    connect_packet->data = NULL;
    // char buffer[buffer_size];
    // bzero(buffer, buffer_size);
    fd_set fd_set_connect;
    do{
        // bzero(buffer, buffer_size);
        FD_ZERO(&fd_set_connect);
        FD_SET(newSocket, &fd_set_connect);

        if (select(newSocket + 1, &fd_set_connect, NULL, NULL, NULL) == -1){
            perror("select");
            return 1;
        }
        if (!FD_ISSET(newSocket, &fd_set_connect)){
            // if (isNewPlayer){
            //     printf("Can't connect\n");
            // } else {
                printf("Can't connect to server\n");
            // }
            close(newSocket);
            return 1;
        }
        // int read_size = recv(newSocket, buffer, buffer_size, MSG_WAITALL);
        //     if (read_size == -1){
        //         perror("recv");
        //         return 1;
        //     }
        //     if (read_size == 0){
        //         printf("Server disconnected\n");
        //         return 1;
        //     }

        if (receive_packet(newSocket, connect_packet) == 0){
            return 1;
        }
        if (connect_packet->type == MSG_CONNECT_START){
            if(isNewPlayer){
                printf("Send new connection request\n");
                init_game_packet(connect_packet, MSG_CONNECT_NEW, 0);
                main_port = port_generator();
                connect_packet -> port = main_port;
            } else {
                init_game_packet(connect_packet, MSG_CONNECT_REQ, 0);
            }
            if (send_game_packet(connect_packet, newSocket) == -1){
                printf("Send connect request failed\n");
                return -1;
            }
            connect_packet->type = MSG_BAD_PORT;
        }
        print_game_packet(connect_packet);
    } while (connect_packet->type == MSG_BAD_PORT);
    if (connect_packet->type == MSG_CONNECT_OK){
        printf("Connected to server\n");
    }

    client *new_client = calloc(1, sizeof(client));
    new_client->socket_send = newSocket;
    new_client->port = port;
    new_client->sockaddr_send = sockAddresse;
    add_client(new_client);
    create_main_listen_socket();
    

}

int waiting_back_connection( client* the_other_client ){
    fd_set fds_connect;
    int max_fd = 0;
    do {
        FD_ZERO(&fds_connect);
        FD_SET( main_listen_sock, &fds_connect);
        max_fd = main_listen_sock;
        if (select(main_listen_sock + 1, &fds_connect, NULL, NULL, NULL) == -1){
            perror("select");
            return 1;
        }
        if (FD_ISSET(main_listen_sock, &fds_connect)){
            accept_back_client(the_other_client);
        }
    } while ();
    
}

int accept_back_client( client* other_cli){
    int size = sizeof(other_cli->sockaddr_listen);
    struct sockaddr* new_sockaddr;
    int new_sock;
    if ( ( new_sock = accept( main_listen_sock, (struct sockaddr *) new_sockaddr, &size) ) <= 0 ){
        perror("accept");
        return -1;
    }
    if ( send_nodata_msg(MSG_CONNECT_BACK_START, new_sock) == -1){
        perror("send");
        return -1;
    }
    game_packet *connect_packet = create_game_packet();
    if (connect_packet == NULL){
        return -1;
    }
    connect_packet->data = NULL;
    if (receive_packet(new_sock, connect_packet) == 0){
        return -1;
    }
    if (conn)

}

int gameListener( int listen_sock ){
    fd_set fd_set_listen;
    int number_fd;
    int max_fd = 0;

    while(1){
        FD_ZERO(&fd_set_listen);
        FD_SET(listen_sock, &fd_set_listen);
        max_fd = listen_sock;

        if (select(max_fd + 1, &fd_set_listen, NULL, NULL, NULL) == -1){
            perror("select");
            return 1;
        }
        if (FD_ISSET(listen_sock, &fd_set_listen)){
            if ( accept_new_client(listen_sock) == NULL){
                printf("Can't accept new player\n");
                return -1;
            }
            printf("Accept and sent\n");
        }
        listen_all_client(&fd_set_listen);
    }
}

int listen_all_client(fd_set *fds){
    client* current_client = first_client();
    game_packet *recv_packet = create_game_packet();
    while( current_client != NULL){
        if(FD_ISSET(current_client->socket_listen, fds)){
            if( receive_packet(recv_packet, current_client->socket_listen) == 0){
                printf("Client disconnected\n");
                close(current_client->socket_listen);
                close(current_client->socket_send);
                client* next_client = current_client->next;
                remove_client(current_client);
                current_client = next_client;
                continue;
            }
            print_game_packet(recv_packet);
            if ( packet_type_handler( current_client, recv_packet) == -1){
                printf("Invalid packet\n");
            }

        }
        current_client = current_client->next;
    }
}

int new_connection( client* cli, game_packet* packet){
    if (port_exist(cli, packet->port)){
        if ( send_nodata_msg(MSG_BAD_PORT, cli->socket_listen) == -1){
            return -1;
        }
        return 0;
    }
    printf("Port is valid\n");
    init_game_packet(packet, MSG_CONNECT_OK, 0);
    send_game_packet(packet, cli->socket_listen);
    return 1;
}

int request_connection( client* cli, game_packet* packet){
    cli -> port = packet->port;
    if ( send_nodata_msg(MSG_CONNECT_OK, cli->socket_listen) == -1){
        return -1;
    }
    connect_back( cli, cli->port);
}

int connect_back(client* cli, int port){
    struct sockaddr_in send_addr = {0};
    game_ip connect_ip = cli->sockaddr_send.sin_addr.s_addr;
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(port);
    send_addr.sin_addr.s_addr = connect_ip;

    int newSocket = socket(AF_INET, SOCK_STREAM, 6);

    if (newSocket == -1){
        perror("socket");
        close(newSocket);
        return -1;
    }
    if ( connect(newSocket, (struct sockaddr *)&cli->sockaddr_send, sizeof(cli->sockaddr_send)) == -1){
        perror("connect");
        close(newSocket);
        return -1;
    }
    game_packet *connect_packet = create_game_packet();
    if (connect_packet == NULL){
        return -1;
    }
    connect_packet->data = NULL;
    fd_set fd_set_connect;
    do{
        // bzero(buffer, buffer_size);
        FD_ZERO(&fd_set_connect);
        FD_SET(newSocket, &fd_set_connect);

        if (select(newSocket + 1, &fd_set_connect, NULL, NULL, NULL) == -1){
            perror("select");
            return 1;
        }
        if (!FD_ISSET(newSocket, &fd_set_connect)){
            printf("Can't connect to server\n");
            close(newSocket);
            return 1;
        if (receive_packet(newSocket, connect_packet) == 0){
            return 1;
        }
        if ( connect_packet->type == MSG_CONNECT_BACK_START){
            if ( connect_packet -> port == port){
                printf("Connected back to client\n");
                return 1;
            } else {
                printf("Port is invalid\n");
                connect_packet->type = MSG_BAD_PORT;
            }
        } else {
            printf("Invalid packet\n");
            return 1;
        }

    } while (connect_packet->type == MSG_BAD_PORT);


}

int packet_type_handler( client* cli, game_packet* packet){
    switch (packet->type)
    {
    case MSG_CONNECT_NEW:
        return new_connection(cli, packet);
    case MSG_CONNECT_REQ:
        return request_connection(cli, packet);
    case MSG_REQ_IP_PORT:
        printf("Sending port\n");
        init_game_packet(packet, MSG_REP_IP_PORT, 4);
        memcpy(packet->data, &main_port, 4);
        send_game_packet(packet, cli->socket_listen);
        return 1;
    case MSG_REP_IP_PORT:
        printf("Received port\n");
        uint32_t port;
        memcpy(&port, packet->data, 4);
        printf("Port: %d\n", port);
        return 1;
        
    default:
        printf("Invalid packet type\n");
        return 0;
    }
}


int main(){
    typedef uint32_t game_ip;
    printf("Enter IP: ");
    scanf("%s", main_ip);
    // printf("Enter port: ");
    // scanf("%d", &main_port);

    main_port = port_generator();
    printf("Port: %d\n", main_port);
    printf("Connecting to server\n");
    int isnewplayer = 1;

    char ip_server[1024];
    int port_server;

    printf("Enter Ip of server: ");
    scanf("%s", ip_server);
    printf("Enter port of server: ");
    scanf("%d", &port_server);
    game_ip ip_ser = inet_addr(ip_server);
    connect_to_game(ip_ser, port_server, isnewplayer);


    // struct sockaddr_in listen_addr = {0};
    // listen_addr.sin_family = AF_INET;
    // listen_addr.sin_port = htons(main_port);
    // listen_addr.sin_addr.s_addr = INADDR_ANY;

    // int listen_sock = socket(AF_INET, SOCK_STREAM, 6);
    // // if (listen_sock == -1){
    // //     perror("socket");
    // //     return 1;
    // // }

    // if (bind( listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)  ) == -1){
    //     perror("bind");
    //     close(listen_sock);
    //     return 1;
    // }
    // if (listen(listen_sock, 5) == -1){
    //     perror("listen");
    //     close(listen_sock);
    //     return 1;
    // }

    // printf("Listening on port %d\n", main_port);

    gameListener();
}

int create_main_listen_socket(){
    struct sockaddr_in listen_addr = {0};
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(main_port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    main_listen_sock = socket(AF_INET, SOCK_STREAM, 6);
    if (main_listen_sock == -1){
        perror("socket");
        return -1;
    }
    if (bind( main_listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)  ) == -1){
        perror("bind");
        close(main_listen_sock);
        return -1;
    }
    if (listen(main_listen_sock, 5) == -1){
        perror("listen");
        close(main_listen_sock);
        return -1;
    }
    printf("Listening on port %d\n", main_port);
    return main_listen_sock;
    

}
