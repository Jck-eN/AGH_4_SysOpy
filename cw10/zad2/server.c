#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "common.h"


int players_count = 0;

pthread_mutex_t players_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char* username;
    int is_active;
    struct sockaddr address;
    int fd;
} player;

player* players[PLAYERS_MAX] = {NULL};

int get_client_index(char* nickname) {
    if(nickname == NULL) return -1;
    for (int i = 0; i < PLAYERS_MAX; i++) {
        if (players[i] != NULL && strcmp(players[i]->username, nickname) == 0) {
            return i;
        }
    }
    return -1;
}

int get_opponent_index(int index) {
    if(index % 2 == 0) return index + 1;
    else return index - 1;
}

int add_player(char* nickname, int fd, struct sockaddr address) {
    if(nickname == NULL) return -1;
    if (get_client_index(nickname) != -1) return -1;

    for (int i = 0; i < PLAYERS_MAX; i++) {
        if (players[i] == NULL) {
            player* new_player = calloc(1, sizeof(player));
            new_player->username = calloc(MESSAGE_LENGTH_MAX, sizeof(char));
            strcpy(new_player->username, nickname);
            new_player->address = address;
            new_player->fd = fd;
            new_player->is_active = 1;

            players[i] = new_player;
            players_count++;

            return i;
        }
    }

    return -1;
}


int init_network_socket(char* port) {
    struct addrinfo* addrinfo;

    struct addrinfo* hints = calloc(1, sizeof(struct addrinfo));
    hints->ai_flags = AI_PASSIVE;
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_family = AF_INET;

    getaddrinfo(NULL, port, hints, &addrinfo);

    int network_socket = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);

    bind(network_socket, addrinfo->ai_addr, addrinfo->ai_addrlen);


    freeaddrinfo(addrinfo);

    return network_socket;
}


int init_local_socket(char* path) {
    int local_socket_desc = socket(AF_UNIX, SOCK_DGRAM, 0);

    struct sockaddr_un* local_socket_addr = calloc(1, sizeof(struct sockaddr_un));
    local_socket_addr->sun_family = AF_UNIX;
    strcpy(local_socket_addr->sun_path, path);

    unlink(path);
    bind(local_socket_desc, (struct sockaddr*) local_socket_addr, sizeof(struct sockaddr_un));

    return local_socket_desc;
}


void remove_player(char* username) {
    printf("Usuwam klienta: %s\n", username);
    int player_index = get_client_index(username);
    if (player_index == -1) return;

    free(players[player_index]->username);
    free(players[player_index]);
    players[player_index] = NULL;
    players_count--;

    int opponent_index = get_opponent_index(player_index);

    if (players[opponent_index] != NULL) {
        printf("Usuwanie przeciwnika rozłączonego gracza\n");
        sendto(players[opponent_index]->fd, "quit: przeciwnik rozłączył się", MESSAGE_LENGTH_MAX, 0,
                &players[opponent_index]->address, sizeof(struct addrinfo));
        free(players[opponent_index]->username);
        free(players[opponent_index]);
        players[opponent_index] = NULL;
        players_count--;
    }
}


void ping_loop() {
    while(1){
        printf("Pinguję klientów\n");
        pthread_mutex_lock(&players_mutex);
        for (int i = 0; i < PLAYERS_MAX; i++) {
            if (players[i] != NULL && !players[i]->is_active) {
                printf("Usuwam nieaktywnego klienta: %s\n", players[i]->username);
                remove_player(players[i]->username);
            }
        }

        for (int i = 0; i < PLAYERS_MAX; i++) {
            if (players[i] != NULL) {
                sendto(players[i]->fd, "ping: ", MESSAGE_LENGTH_MAX, 0,
                        &players[i]->address, sizeof(struct addrinfo));
                players[i]->is_active = 0;
            }
        }
        pthread_mutex_unlock(&players_mutex);

        sleep(5);
    }
}


int poll_both_sockets(int local_socket, int network_socket) {
    struct pollfd* socket_fd = calloc(2 + players_count, sizeof(struct pollfd));
    socket_fd[0].events = POLLIN;
    socket_fd[0].fd = local_socket;
    socket_fd[1].events = POLLIN;
    socket_fd[1].fd = network_socket;


    poll(socket_fd, 2, -1);

    int result = -1;
    for (int i = 0; i < 2; i++) {
        if (socket_fd[i].revents & POLLIN) {
            result = socket_fd[i].fd;
            break;
        }
    }

    free(socket_fd);

    return result;
}

void clear(){
    for(int i=0; i< PLAYERS_MAX; i++){
        if(players[i] != NULL && players[i]->is_active){
            remove_player(players[i]->username);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Nieprawidłowe wywołanie programu:\n");
        printf("---\n");
        printf("Użycie:\n");
        printf("server <port> <path>\n");
        printf("---\n");
        printf("port - nr portu na którym działa serwer (np. 2345)\n");
        printf("path - ścieżka lokalnego socketa (np. /tmp/socket)\n");
        exit(1);
    }

    atexit(clear);

    char* port = argv[1];
    char* socket_path = argv[2];

    srand(time(NULL));

    int local_socket = init_local_socket(socket_path);
    int network_socket = init_network_socket(port);

    pthread_t t;
    pthread_create(&t, NULL, (void *(*)(void *)) ping_loop, NULL);

    while (1) {
        int sock_fd = poll_both_sockets(local_socket, network_socket);

        char buffer[MESSAGE_LENGTH_MAX + 1];
        struct sockaddr from_addr;
        socklen_t from_length = sizeof(struct sockaddr);
        recvfrom(sock_fd, buffer, MESSAGE_LENGTH_MAX, 0, &from_addr, &from_length);


        printf("%s\n", buffer);
        if(buffer[0] == '\0') perror("Brak odpowiedzi");
        char* cmd = strtok(buffer, ":");
        char* arg = strtok(NULL, ":");
        char* username = strtok(NULL, ":");

        pthread_mutex_lock(&players_mutex);

        if (strcmp(cmd, "add") == 0) {
            int index = add_player(username, sock_fd, from_addr);

            if (index == -1) {
                sendto(sock_fd, "add:name_taken", MESSAGE_LENGTH_MAX, 0,  (struct sockaddr*)&from_addr, sizeof(struct addrinfo));
            }
            else if (index % 2 == 0) {
                sendto(sock_fd, "add:no_enemy", MESSAGE_LENGTH_MAX, 0,  (struct sockaddr*)&from_addr, sizeof(struct addrinfo));
            }
            else {
                int first_waiting = rand() % 2;
                int first_player_index = index - first_waiting;
                int second_player_index = get_opponent_index(first_player_index);

                sendto(players[first_player_index]->fd, "add:O",
                     MESSAGE_LENGTH_MAX, 0, &players[first_player_index]->address, sizeof(struct addrinfo));
                sendto(players[second_player_index]->fd, "add:X",
                     MESSAGE_LENGTH_MAX, 0, &players[second_player_index]->address, sizeof(struct addrinfo));
            }
        }
        if (strcmp(cmd, "move") == 0) {
            int move = atoi(arg);
            int player = get_client_index(username);

            sprintf(buffer, "move:%d", move);
            sendto(players[get_opponent_index(player)]->fd, buffer, MESSAGE_LENGTH_MAX,0,
                    &players[get_opponent_index(player)]->address, sizeof(struct addrinfo));
        }
        if (strcmp(cmd, "quit") == 0) {
            remove_player(username);
        }
        if (strcmp(cmd, "pong") == 0) {
            int player = get_client_index(username);
            if (player != -1) {
                players[player]->is_active = 1;
            }
        }
        pthread_mutex_unlock(&players_mutex);
    }
}