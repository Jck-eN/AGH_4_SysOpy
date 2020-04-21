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


int s_queue = -1;
int c_queue = -1;
int f_queue = -1;
int f_pid = -1;

int c_ID;

int queue_is_empty(int queue)
{
    struct msqid_ds buf;
    msgctl(queue, IPC_STAT, &buf);

    return buf.msg_qnum == 0;
}


void init() {

    printf("KOLEJKA KLIENTA: %i\n\n\n", c_queue);
    printf("----- SYSTEM V - CHAT -----\n");
    printf("---------------------------\n");
    message msg;
    char content[MESSAGE_MAX];
    msg.m_type = INIT;
    sprintf(content, "%i", c_queue);
    strcpy(msg.msg, content);
    msg.sender = getpid();
    msgsnd(s_queue, &msg, MAX_MESSAGE_SIZE, 0);

    msgrcv(c_queue, &msg, MAX_MESSAGE_SIZE, -10, 0);
    c_ID = atoi(msg.msg);
    printf("Klient otrzymał ID: %d \n", c_ID);
    printf("---------------------------\n");
}

void send_server(long type, char *content) {
    message msg;
    msg.m_type = type;
    strcpy(msg.msg, content);
    msg.sender = c_ID;

    msgsnd(s_queue, &msg, sizeof(message), IPC_NOWAIT);
}

void send_friend(long type, char *content) {
    if(strcmp(content, "") == 0 || strcmp(content, "\n") == 0) return;
    message msg;
    msg.m_type = type;
    strcpy(msg.msg, content);
    msg.sender = c_ID;
    if(f_queue>=0) {
        msgsnd(f_queue, &msg, MAX_MESSAGE_SIZE, IPC_NOWAIT);
        kill(f_pid, SIGRTMIN);
    }
    else printf("Drugi klient nie jest podłączony!\n");
}



void quit_client() {
    msgctl(c_queue, IPC_RMID, NULL);
    exit(0);
}

void send_stop() {
    if(c_queue != -1) send_server(STOP, "");
    quit_client();
}

void receive_int(int signal){
    exit(1);
}

void send_split_command(char *full_command, char* first, char* message){
    {
        char line_copy[MESSAGE_MAX];
        strcpy(line_copy, full_command);
        char *line_ptr = line_copy;

        char *command = strtok_r(line_copy, " \n", &line_ptr);

        char *split_message = strtok_r(NULL, "\n", &line_ptr);

        if (command == NULL){
            first[0] = '\0';
        }
        else{
            strcpy(first, command);
        }
        if (split_message == NULL){
            message[0] = '\0';
        }
        else{
            strcpy(message, split_message);
        }
    }
}

void send_parse_command(char *input){
    char msg_text[MESSAGE_MAX];
    char command[20];

    send_split_command(input, command, msg_text);
    if(strcmp(command, "LIST") == 0){
        send_server(LIST, "");
    }
    else if(strcmp(command, "CONNECT") == 0){
        send_server(CONNECT, msg_text);
    }
    else if(strcmp(command, "DISCONNECT") == 0){
        send_server(DISCONNECT, "");
        printf("Zakończono chat z procesem o PID: %d\n", f_pid);
        printf("---------------------------------\n");
        f_queue = -1;
        f_pid = -1;
    }
    else if(strcmp(command, "STOP") == 0){
        send_stop();
    }
    else {
        send_friend(TEXT, input);
    }
}



void receive_message(){
    signal(SIGINT, receive_int);
    while(!queue_is_empty(c_queue)){
        message msg;
        int rec =msgrcv(c_queue, &msg, MAX_MESSAGE_SIZE, -10, IPC_NOWAIT);
        if(rec != -1){
            switch (msg.m_type){
                case STOP:
                    quit_client();
                    break;
                case CONNECT: {
                    int friend_q, friend_p;
                    sscanf(msg.msg, "%d %d", &friend_q, &friend_p);
                    if (friend_q >=0){
                        f_queue = friend_q;
                        f_pid = friend_p;
                        printf("Rozpoczęto chat z procesem o PID %d:\n", f_pid);
                        printf("----------------------------------------\n");
                    }
                    else{
                        perror("Nie można nawiązać połączenia z klientem");
                    }
                    break;
                }
                case TEXT:
                    printf(">> %s", msg.msg);
                    break;

                case DISCONNECT:
                    printf("Zakończono chat z procesem o PID: %d\n", f_pid);
                    printf("----------------------------------------\n");
                    f_queue = -1;
                    f_pid = -1;
                    break;
                default:
                    break;
            }
        }
    }
}


void receive_message_signal(){
    receive_message();
}


int main() {
    atexit(send_stop);

    signal(SIGRTMIN, receive_message_signal);

    signal(SIGINT, receive_int);

    char* homepath = getenv("HOME");
    key_t s_queue_key = ftok(homepath, PROJECT_ID);
    key_t c_queue_key = ftok(homepath, getpid());

    if((s_queue = msgget(s_queue_key,  IPC_CREAT  | 0666)) == -1) {
        perror("Nie udało się otworzyć kolejki serwera");
        return -1;
    }
    if((c_queue = msgget(c_queue_key, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        perror("Nie udało się otworzyć kolejki klienta");
        return -1;
    }
    init();

    char m_buff[MESSAGE_MAX];
    while(true){
        fgets(m_buff, MESSAGE_MAX-1, stdin);
        send_parse_command(m_buff);
        receive_message();
    }

}
