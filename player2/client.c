#include "client.h"

client* accept_new_client( int listen_sock){
    client *new_client = calloc(1, sizeof(client));
    new_client->id = 0;
    printf("Waiting for new player\n");
    int size = sizeof(new_client->sockAddresse);
    if ( (new_client->socket = accept(listen_sock, (struct sockaddr *)&new_client->sockAddresse, &size)) <= 0){
        perror("accept");
        return NULL;
    }
    printf("New player connected with socket %d \n", new_client->socket);
    char buffer[buffer_size];
    bzero(buffer, buffer_size);
    strcpy(buffer, "Welcome to the game\n");
    if (send(new_client->socket, buffer, buffer_size-1, 0) == -1){
        perror("send");
        return NULL;
    }
    return new_client;
}

int connect_to_game(game_ip ip, int isNewPlayer){
    struct sockaddr_in sockAddresse ={0};
    sockAddresse.sin_family = AF_INET;
    sockAddresse.sin_port = htons(PORT);
    sockAddresse.sin_addr.s_addr = ip;

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
    char buffer[buffer_size];
    bzero(buffer, buffer_size);

    fd_set fd_set_connect;
    do{
        bzero(buffer, buffer_size);
        FD_ZERO(&fd_set_connect);
        FD_SET(newSocket, &fd_set_connect);

        if (select(newSocket + 1, &fd_set_connect, NULL, NULL, NULL) == -1){
            perror("select");
            return 1;
        }
        if (!FD_ISSET(newSocket, &fd_set_connect)){
            if (isNewPlayer){
                printf("Can't connect\n");
            } else {
                printf("Can't connect to server\n");
            }
            close(newSocket);
            return 1;
        }
        int read_size = recv(newSocket, buffer, buffer_size, MSG_WAITALL);
            if (read_size == -1){
                perror("recv");
                return 1;
            }
            if (read_size == 0){
                printf("Server disconnected\n");
                return 1;
            }
        printf("All player: %s\n", buffer);
        break;
    } while (1);
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
    game_ip ip = inet_addr(IP);
    printf("Connecting to server %s\n", IP);
    connect_to_game(ip, 1);


    struct sockaddr_in listen_addr = {0};
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(PORT);
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

    printf("Listening on port %d\n", PORT);

    gameListener(listen_sock);
}

