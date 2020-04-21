//
// Created by jacek on 14.04.2020.
//

#ifndef SYSOPY_06_1_HELPER_H
#define SYSOPY_06_1_HELPER_H

#define MESSAGE_MAX 600
#define CLIENTS_MAX 10
#define MESSAGES_MAX 10

#define STOP 6
#define DISCONNECT 5
#define LIST 4
#define CONNECT 3
#define INIT 2
#define TEXT 1

#define S_FREE 1
#define S_CHAT 2
#define S_DISCONNECTED 3

typedef struct message {
    long m_type;
    char msg[MESSAGE_MAX];
    pid_t sender;
} message;

typedef struct client {
    int c_queue;
    pid_t pid;
    int status;
    int friend;
} client;
const char server_path[] = "/server";

#define ALL_MESSAGE_SIZE sizeof(message)

#endif //SYSOPY_06_1_HELPER_H
