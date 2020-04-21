#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>


client clients[CLIENTS_MAX];
int clients_no=0;
int s_queue;

void change_status(int client_id, int new_status){
    if(client_id>=CLIENTS_MAX || client_id<0){
        printf("Nieprawidłowa zmiana statusu!");
        return;
    }
    clients[client_id].status = new_status;
}


void send_to_client(long type, char *content, int c_id) {
    if(c_id >= CLIENTS_MAX || c_id < 0 || clients[c_id].c_queue < 0) {
        return;
    }

    message msg;
    msg.sender = -1;

    msg.m_type = type;
    strcpy(msg.msg, content);

    msgsnd(clients[c_id].c_queue, &msg, MAX_MESSAGE_SIZE, 0);
    if(type!=INIT) kill(clients[c_id].pid, SIGRTMIN);
}

void received_init(int c_pid, char *msg) {
    int id;
    for (id = 0; id < CLIENTS_MAX; id++) {
        if(clients[id].status == S_DISCONNECTED)
            break;
    }

    if(id >= CLIENTS_MAX)
        return;

    int client_queue = -1;
    sscanf(msg, "%d", &client_queue);
    if(client_queue < 0)
        return;

    clients[id].c_queue = client_queue;
    clients[id].pid = c_pid;
    change_status(id, S_FREE);

    printf("Nowy klient z ID: %d \n", id);

    char msg_client_id[MESSAGE_MAX];
    sprintf(msg_client_id, "%d", id);
    send_to_client(INIT, msg_client_id, id);
    clients_no++;
}

void received_list(int client_id){
    char response[MESSAGE_MAX] = "\n";
    char tmp[50];
    for(int i=0; i<CLIENTS_MAX; i++){
        if(1){
            char status_name[20];
            switch(clients[i].status){
                case S_DISCONNECTED: {
                    strcpy(status_name, "-");
                    break;
                }
                case S_FREE: {
                    strcpy(status_name, "Dostępny");
                    break;
                }
                case S_CHAT: {
                    strcpy(status_name, "Zajęty");
                    break;
                }
                default:{
                    strcpy(status_name, "Błąd");
                    break;
                }
            }
            sprintf(tmp, "%d. %s\n", i, status_name);
            strcat(response, tmp);
        }
    }
    send_to_client(TEXT, response, client_id);
}

void received_connect(int client_id, char* friend){
    int friend_id = atoi(friend);
    if(friend_id < 0 || friend_id >= CLIENTS_MAX || strcmp(friend, "")==0 || (friend_id == 0 && (strcmp(friend, "0")!=0))){
        send_to_client(TEXT, "Nieprawidłowy numer klienta!\n", client_id);
        return;
    }
    char response[MESSAGE_MAX];
    if(client_id == friend_id){
        send_to_client(TEXT, "Nie możesz połączyć się ze sobą samym!\n", client_id);
        return;
    }
    else if(clients[client_id].status == S_FREE && clients[friend_id].status == S_FREE){
        clients[client_id].friend = friend_id;
        change_status(client_id, S_CHAT);
        clients[friend_id].friend = client_id;
        change_status(friend_id, S_CHAT);
        sprintf(response, "%d %d", clients[friend_id].c_queue, clients[friend_id].pid);
        send_to_client(CONNECT, response, client_id);
        sprintf(response, "%d %d", clients[client_id].c_queue, clients[client_id].pid);
        send_to_client(CONNECT, response, friend_id);
    }
    else{
        send_to_client(TEXT, "Podłączenie niemożliwe z powodu statusu klienta!\n", client_id);
        return;
    }
}

void received_disconnect(int client_id){
    if(clients[client_id].status == S_CHAT ){
        int friend_id = clients[client_id].friend;
        send_to_client(DISCONNECT, "", friend_id);
        change_status(client_id, S_FREE);
        clients[client_id].friend = -1;
        if(clients[friend_id].status == S_CHAT) {
            change_status(friend_id, S_FREE);
            clients[friend_id].friend = -1;
        }
    }
}

void received_stop(int client_id){
    if(clients[client_id].status == S_CHAT ){
        int friend_id = clients[client_id].friend;
        send_to_client(DISCONNECT, "", friend_id);
        if(clients[friend_id].friend == client_id){
            change_status(friend_id, S_FREE);
            clients[friend_id].friend = -1;
        }
    }
    change_status(client_id, S_DISCONNECTED);
    clients[client_id].pid = -1;
    clients[client_id].friend = -1;
    clients[client_id].c_queue = -1;
}

void quit(int sig_number) {
    for (int i = 0; i < CLIENTS_MAX; i++) {
        if(clients[i].status != S_DISCONNECTED) {
            send_to_client(STOP, "", i);
        }
    }
    msgctl(s_queue, IPC_RMID, NULL);
    printf("Usunięto kolejkę serwera\n");
    exit(0);
}

void receive_int(int signal){
    exit(0);
}

void msg_handler(message received){
    switch(received.m_type){
        case TEXT:
            printf("Otrzymano wiadomość tekstową: %d: %s\n", received.sender, received.msg);
            break;
        case INIT:
            received_init(received.sender, received.msg);
            break;
        case LIST:
            received_list(received.sender);
            break;
        case CONNECT:
            received_connect(received.sender, received.msg);
            break;
        case DISCONNECT:
            received_disconnect(received.sender);
            break;
        case STOP:
            received_stop(received.sender);
            break;
    }
}

int main() {
    atexit(quit);
    char* homepath = getenv("HOME");
    key_t s_queue_key = ftok(homepath, PROJECT_ID);

    for(int i = 0; i < CLIENTS_MAX; i++) {
        clients[i].c_queue = -1;
        clients[i].status = S_DISCONNECTED;
        clients[i].friend = -1;
        clients[i].pid = -1;
    }

    signal(SIGINT, receive_int);

    if((s_queue = msgget(s_queue_key, IPC_CREAT | 0666)) == -1) {
        perror("Nie udało się utworzyć kolejki serwera");
        return -1;
    }
    printf("KOLEJKA SERWERA: %d\n", s_queue);
    printf("----------------------------\n");
    message received;
    while(1) {
        msgrcv(s_queue, &received, sizeof(message), -10, 0) == -1;
        msg_handler(received);
    }
}
