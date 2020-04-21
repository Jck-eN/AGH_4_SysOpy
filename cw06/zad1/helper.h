#ifndef SYSOPY_06_1_HELPER_H
#define SYSOPY_06_1_HELPER_H

#define PROJECT_ID 'J'
#define MESSAGE_MAX 600
#define CLIENTS_MAX 10

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define TEXT 6

#define S_FREE 1
#define S_CHAT 2
#define S_DISCONNECTED 3

typedef struct message {
    long m_type;
    pid_t sender;
    char msg[MESSAGE_MAX];
} message;

typedef struct client {
    int c_queue;
    pid_t pid;
    int status;
    int friend;
} client;

#define ALL_MESSAGE_SIZE sizeof(message)
#define MAX_MESSAGE_SIZE sizeof(message)-sizeof(long)

#endif //SYSOPY_06_1_HELPER_H
