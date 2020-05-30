#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <stdio.h>

#include "common.h"

int server_socket;
int is_o;
char buffer[MESSAGE_LENGTH_MAX + 1];
char* username;

board_t board;

typedef enum {
    START,
    WAIT_FOR_OTHER_PLAYER_CONNECTION,
    WAIT_FOR_MOVE,
    OPPONENT_MADE_MOVE,
    MOVE,
    QUIT
} state_t;

state_t state = START;

char *command, *argument;
struct sockaddr_un* local_socket_addr;

pthread_mutex_t mutex_reply = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reply_cond_mutex = PTHREAD_COND_INITIALIZER;


void draw_board() {
    char* symbols[3] = {" \r ", " O ", " X "};
    for (int y = 0; y < 3; y++) {
        printf("|");
        for (int x = 0; x < 3; x++) {
            int field_number = 3*y+x+1;
            char buf[4];
            sprintf(buf, " %d ", field_number);
            symbols[0] = buf;
            printf("%s|", symbols[board.fields[field_number-1]]);
        }
        printf("\n");
        if(y<2) printf("----+---+----\n");
    }
}


void board_check() {
    // check for a win
    int is_won = 0;
    field winner = get_winner(&board);
    if (winner != EMPTY) {
        if ((is_o && winner == O) || (!is_o && winner == X)) {
            printf("Gra skończona. Wygrałeś!\n");
        } else {
            printf("Gra skończona. Przegrałeś!\n");
        }

        is_won = 1;
    }

    int is_drawn = 1;
    for (int i = 0; i < 9; i++) {
        if (board.fields[i] == EMPTY) {
            is_drawn = 0;
            break;
        }
    }

    if (is_drawn && !is_won) {
        printf("Gra skończona. Remis!\n");
    }

    if (is_won || is_drawn) {
        sleep(1);
        state = QUIT;
    }
}


void game_over() {
    char buffer[MESSAGE_LENGTH_MAX + 1];
    sprintf(buffer, "quit: :%s", username);
    send(server_socket, buffer, MESSAGE_LENGTH_MAX, 0);
    exit(0);
}


void send_to_server(char* message){
    send(server_socket, message, MESSAGE_LENGTH_MAX, 0);
}


void clear(){
    if(local_socket_addr!= NULL) free(local_socket_addr);
}


void game_loop() {
    switch(state) {
        case START:
            if (strcmp(argument, "name_taken") == 0) {
                printf("Ta nazwa użytkownika już istnieje. Wybierz inną!");
                exit(1);
            }
            else if (strcmp(argument, "no_enemy") == 0) {
                printf("Oczekiwanie na podłączenie przeciwnika...\n");
                state = WAIT_FOR_OTHER_PLAYER_CONNECTION;
            }
            else {
                board = init_new_board();
                is_o = argument[0] == 'X';
                if(is_o) {
                    state = MOVE;
                }
                else state = WAIT_FOR_MOVE;
            }
            break;
        case WAIT_FOR_OTHER_PLAYER_CONNECTION:
            pthread_mutex_lock(&mutex_reply);

            while (state != QUIT && state != START) {
                pthread_cond_wait(&reply_cond_mutex, &mutex_reply);
            }

            pthread_mutex_unlock(&mutex_reply);

            board = init_new_board();
            is_o = argument[0] == 'X';
            if(is_o) {
                state = MOVE;
            }
            else state = WAIT_FOR_MOVE;
            break;
        case WAIT_FOR_MOVE:
            printf("Oczekiwanie na ruch przeciwnika...\n");

            pthread_mutex_lock(&mutex_reply);

            while (state != QUIT && state != OPPONENT_MADE_MOVE) {
                pthread_cond_wait(&reply_cond_mutex, &mutex_reply);
            }

            pthread_mutex_unlock(&mutex_reply);

            break;

        case OPPONENT_MADE_MOVE:
            printf("Przeciwnik wykonał ruch:");
            int move = atoi(argument);
            printf(" %d\n", move+1);
            make_move(&board, move);
            board_check();
            if (state != QUIT) {
                state = MOVE;
            }
            break;

        case MOVE:
            draw_board();

            int field_number;
            do {
                printf("Podaj następny ruch [%c]: ", is_o ? 'X' : 'O');
                scanf("%d", &field_number);
                field_number--;
            } while (!make_move(&board, field_number));

            draw_board();

            char local_buff[MESSAGE_LENGTH_MAX + 1];

            sprintf(local_buff, "move:%d:%s", field_number, username);
            send_to_server(local_buff);

            board_check();
            if (state != QUIT) {
                state = WAIT_FOR_MOVE;
            }
            break;
        case QUIT:
            game_over();
            break;
        default:
            break;
    }
    game_loop();
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Nieprawidłowe wywołanie programu:\n");
        printf("---\n");
        printf("Użycie:\n");
        printf("client <nick> <local|remote> <address>\n");
        printf("---\n");
        printf("address - jeżeli [local] to lokalny adres socketa (np. tmp/socket)\n");
        printf("          jeżeli [remote] to adres i port socketa (np. 127.0.0.1:2345)\n");
        exit(1);
    }

    atexit(clear);

    username = argv[1];
    char* connection_mode = argv[2];
    char* server_address = argv[3];

    signal(SIGINT, game_over);
    signal(SIGPIPE, game_over);

    if (strcmp(connection_mode, "local") == 0) {
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

        local_socket_addr = calloc(1, sizeof(struct sockaddr_un));
        local_socket_addr->sun_family = AF_UNIX;
        strcpy(local_socket_addr->sun_path, server_address);

        connect(server_socket, (struct sockaddr*) local_socket_addr,
                sizeof(struct sockaddr_un));
    }
    else {

        struct addrinfo* hints = calloc(1, sizeof(struct addrinfo));
        struct addrinfo* addrinfo;
        hints->ai_socktype = SOCK_STREAM;
        hints->ai_family = AF_UNSPEC;

        char* address = strtok(server_address, ":");
        char* port = strtok(NULL, ":");

        getaddrinfo(address, port, hints, &addrinfo);
        if(addrinfo == NULL){
            perror("Nie udało się znaleźć socketu!");
            exit(1);
        }
        server_socket = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);

        connect(server_socket, addrinfo->ai_addr, addrinfo->ai_addrlen);

        freeaddrinfo(addrinfo);
        free(hints);
    }
    char msg_buffer[MESSAGE_LENGTH_MAX + 1];
    sprintf(msg_buffer, "add: :%s", username);
    send_to_server(msg_buffer);

    int game_thread_running = 0;

    while (1) {
        recv(server_socket, msg_buffer, MESSAGE_LENGTH_MAX, 0);
        command = strtok(msg_buffer, ":");
        argument = strtok(NULL, ":");

        pthread_mutex_lock(&mutex_reply);
        if (strcmp(command, "add") == 0) {
            state = START;
            if (!game_thread_running) {
                pthread_t thread;
                pthread_create(&thread, NULL, (void* (*)(void*))game_loop, NULL);
                game_thread_running = 1;
            }
        }
        else if (strcmp(command, "ping") == 0) {
            sprintf(msg_buffer, "pong: :%s", username);
            send_to_server(msg_buffer);
        }
        else if (strcmp(command, "move") == 0) {
            state = OPPONENT_MADE_MOVE;
        }
        else if (strcmp(command, "quit") == 0) {
            state = QUIT;
            printf("%s\n", argument);
            exit(0);
        }

        pthread_cond_signal(&reply_cond_mutex);
        pthread_mutex_unlock(&mutex_reply);
    }
}