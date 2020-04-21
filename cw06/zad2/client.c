#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>


int s_queue = -1;
int c_queue = -1;
int f_queue = -1;
int f_pid = -1;
int c_ID;

void init() {

    printf("-------- POSIX CHAT --------\n");
    printf("----------------------------\n");
    message msg;
    char content[MESSAGE_MAX];
    msg.m_type = INIT;
    sprintf(content, "%i", getpid());
    strcpy(msg.msg, content);
    msg.sender = getpid();
    if(mq_send(s_queue, &msg, ALL_MESSAGE_SIZE, msg.m_type) == -1){
        printf("Nie udało się wysłać rejestracji!");
        exit(-1);
    }
    message msg2;
    if(mq_receive(c_queue, &msg2, ALL_MESSAGE_SIZE, NULL) ==-1){
        printf("Nie udało się odebrać rejestracji!");
        exit(-1);
    }
    c_ID = atoi(msg2.msg);
    printf("Klient otrzymał ID: %d \n", c_ID);
    printf("----------------------------\n");
}

void send_server(long type, char *content) {
    message msg;
    msg.m_type = type;
    strcpy(msg.msg, content);
    msg.sender = c_ID;

    mq_send(s_queue, &msg, ALL_MESSAGE_SIZE, msg.m_type);
}

void send_friend(long type, char *content) {

    if(strcmp(content, "") == 0 || strcmp(content, "\n") == 0) return;
    message msg;
    msg.m_type = type;
    strcpy(msg.msg, content);
    msg.sender = c_ID;
    if(f_queue>=0) {
        mq_send(f_queue, &msg, ALL_MESSAGE_SIZE, msg.m_type);
        kill(f_pid, SIGRTMIN);
    }
    else printf("Drugi klient nie jest podłączony!\n");
}



void quit_client() {
    if(c_queue != -1 && s_queue != -1) send_server(STOP, "");
    if(f_queue != -1) mq_close(f_queue);
    if(s_queue != -1) mq_close(s_queue);
    mq_close(c_queue);
    char client_q_name[20];
    sprintf(client_q_name, "/%d", getpid());
    usleep(100000);
    mq_unlink(client_q_name);
}

void send_stop() {
    exit(0);
}

void receive_int(int signal){
    exit(0);
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
        if(f_queue != -1) mq_close(f_queue);
        f_queue = -1;
        f_pid = -1;

        printf("Zakończono chat\n");
        printf("---------------\n");
    }
    else if(strcmp(command, "STOP") == 0){
        send_stop();
    }
    else {
        send_friend(TEXT, input);
    }
}



void receive_message(){
    message msg;
    int rec = mq_receive(c_queue, (char*) &msg, ALL_MESSAGE_SIZE, NULL);
    switch (msg.m_type){
        case STOP:
            quit_client();
            break;
        case CONNECT: {
            char friend_q_name[20];
            sprintf(friend_q_name, "/%s", msg.msg);
            f_pid = atoi(msg.msg);
            printf("Rozpoczęto chat z procesem o PID %d:\n", f_pid);
            printf("----------------------------------------\n");
            if((f_queue = mq_open(friend_q_name, O_WRONLY)) == -1){
                perror("Nie można nawiązać połączenia z klientem");
            }
            break;
        }
        case TEXT:
            printf(">> %s", msg.msg);
            break;

        case DISCONNECT:
            mq_close(f_queue);
            printf("Zakończono chat\n");
            printf("---------------\n");
            f_queue = -1;
            f_pid = -1;
            break;
        default:
            break;
    }
}


void receive_message_signal(){
    receive_message();
}


int main() {
    atexit(quit_client);
    printf("PID: %d\n", getpid());

    signal(SIGRTMIN, receive_message_signal);
    signal(SIGINT, receive_int);

    char client_q_name[20];
    sprintf(client_q_name, "/%d", getpid());

    if((s_queue = mq_open(server_path, O_WRONLY)) == -1) {
        perror("Nie udało się otworzyć kolejki serwera");
        return -1;
    }

    struct mq_attr q_attr;
    q_attr.mq_msgsize = ALL_MESSAGE_SIZE;
    q_attr.mq_maxmsg = MESSAGES_MAX;

    if((c_queue = mq_open(client_q_name, O_RDONLY | O_CREAT , 0666, &q_attr)) == -1){
        perror("Nie udało się otworzyć kolejki klienta");
        return -1;
    }
    init();

    char m_buff[MESSAGE_MAX];
    while(1){
        fgets(m_buff, MESSAGE_MAX-1, stdin);
        send_parse_command(m_buff);
    }

}
