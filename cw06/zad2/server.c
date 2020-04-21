#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>


client clients[CLIENTS_MAX];
int clients_no=0;
mqd_t s_queue;

void change_client_status(int client_id, int new_status){
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

    static message msg;
    msg.sender = -1;

    msg.m_type = type;
    strcpy(msg.msg, content);
    mq_send(clients[c_id].c_queue, &msg, ALL_MESSAGE_SIZE, msg.m_type);
    if(type!=INIT){
        kill(clients[c_id].pid, SIGRTMIN);
    }
}

void received_init(int c_pid, char *msg) {
    int id;
    for (id = 0; id < CLIENTS_MAX; id++) {
        if(clients[id].status == S_DISCONNECTED)
            break;
    }

    if(id >= CLIENTS_MAX)
        return;

    char client_q_name[20];
    sprintf(client_q_name, "/%d", c_pid);
    int client_queue = mq_open(client_q_name, O_WRONLY);
    if(client_queue == -1) {
        return;
    }
    clients[id].c_queue = client_queue;
    clients[id].pid = c_pid;
    change_client_status(id, S_FREE);

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
    if(friend_id < 0 || friend_id >= CLIENTS_MAX || strcmp(friend, "")==0){
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
        change_client_status(client_id, S_CHAT);
        clients[friend_id].friend = client_id;
        change_client_status(friend_id, S_CHAT);
        sprintf(response, "%d",clients[friend_id].pid);
        send_to_client(CONNECT, response, client_id);
        sprintf(response, "%d", clients[client_id].pid);
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
        change_client_status(client_id, S_FREE);
        clients[client_id].friend = -1;
        if(clients[friend_id].status == S_CHAT) {
            change_client_status(friend_id, S_FREE);
            clients[friend_id].friend = -1;
        }
    }
}

void received_stop(int client_id){
    if(clients[client_id].status == S_CHAT ){
        int friend_id = clients[client_id].friend;
        send_to_client(DISCONNECT, "", friend_id);
        if(clients[friend_id].friend == client_id){
            change_client_status(friend_id, S_FREE);
            clients[friend_id].friend = -1;
        }
    }
    mq_close(clients[client_id].c_queue);
    change_client_status(client_id, S_DISCONNECTED);
    clients[client_id].pid = -1;
    clients[client_id].friend = -1;
    clients[client_id].c_queue = -1;
}

void quit_server() {
    for (int i = 0; i < CLIENTS_MAX; i++) {
        if(clients[i].status != S_DISCONNECTED) {
            mq_close(clients[i].c_queue);
            kill(clients[i].pid, SIGINT);
        }
    }
    usleep(100000);
    mq_close(s_queue);
    mq_unlink(server_path);
    printf("Usunięto kolejkę serwera\n");
    exit(0);
}

void receive_int(int signal){
    exit(0);
}

void received_message_handler(message received){
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
    atexit(quit_server);

    for(int i = 0; i < CLIENTS_MAX; i++) {
        clients[i].c_queue = -1;
        clients[i].status = S_DISCONNECTED;
        clients[i].friend = -1;
        clients[i].pid = -1;
    }

    signal(SIGINT, receive_int);

    struct mq_attr q_attr;
    q_attr.mq_msgsize = ALL_MESSAGE_SIZE;
    q_attr.mq_maxmsg = MESSAGES_MAX;


    if((s_queue = mq_open(server_path,O_RDONLY | O_CREAT, 0666, &q_attr)) == -1) {
        perror("Nie udało się utworzyć kolejki serwera");
        return -1;
    }
    printf("--- SERWER CHATU AKTYWNY --- %d\n", s_queue);
    printf("----------------------------\n");
    message received;
    while(1) {
        if(mq_receive(s_queue, &received, ALL_MESSAGE_SIZE, NULL) == -1)
            perror("Nie udało się odebrać wiadomości");
        received_message_handler(received);
    }
}
