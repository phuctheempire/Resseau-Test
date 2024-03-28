#include "client.h"

static int main_port;
static char main_ip [1024];

client* accept_new_client( int listen_sock){
    client *new_client = calloc(1, sizeof(client));
    printf("Waiting for new player\n");
    int size = sizeof(new_client->sockaddr_client);
    if ( (new_client->socket_listen = accept(listen_sock, (struct sockaddr *)&new_client->sockaddr_client, &size)) <= 0){
        perror("accept");
        return NULL;
    }
    printf("New player connected with socket %d \n", new_client->socket_listen);
    char buffer[buffer_size];
    bzero(buffer, buffer_size);
    strcpy(buffer, "Welcome to the game\n");
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

int connect_to_game( int isNewPlayer){
    struct sockaddr_in sockAddresse ={0};
    char ip[1024];
    int port;
    printf("Enter Ip of server: ");
    scanf("%s", ip);
    printf("Enter port of server: ");
    scanf("%d", &port);
    game_ip connect_ip = inet_addr(ip);
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
}

int send_packet( int socket, struct packet *packet){

    int send_size = send(socket, packet, sizeof(struct packet), 0);
    if (send_size == -1){
        perror("send");
        return 1;
    }
    return 0;
}

int receive_packet(int socket, struct packet *packet){
    int read_size = recv(socket, packet, sizeof(struct packet), 0);
    if (read_size == -1){
        perror("recv");
        return 1;
    }
    if (read_size == 0){
        printf("Server disconnected\n");
        return 1;
    }
    return 0;
}

int gameListener( int listen_sock){
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
        }
        printf("Accept and sent\n");
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
    connect_to_game(isnewplayer);


    struct sockaddr_in listen_addr = {0};
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(main_port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    int listen_sock = socket(AF_INET, SOCK_STREAM, 6);
    // if (listen_sock == -1){
    //     perror("socket");
    //     return 1;
    // }

    if (bind( listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)  ) == -1){
        perror("bind");
        close(listen_sock);
        return 1;
    }
    if (listen(listen_sock, 5) == -1){
        perror("listen");
        close(listen_sock);
        return 1;
    }

    printf("Listening on port %d\n", main_port);

    gameListener(listen_sock);
}

