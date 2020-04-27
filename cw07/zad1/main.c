#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "helper.h"

pid_t pid = -1;
char current_time[100];
int shared_memory_id = -1;
shop_queue* shared_memory = NULL;
int sem_id = -1;
pid_t pids[WORKER_1+WORKER_2+WORKER_3];

void change_sem(int sem_num, int sem_diff){
    short op = (short) sem_diff;
    if(sem_id != -1){
        struct sembuf sem_request;
        sem_request.sem_num = sem_num;
        sem_request.sem_op = op;
        sem_request.sem_flg = 0;

        if (semop(sem_id, &sem_request, 1))
            printf("Nie udało się zmienić wartości semafora!\n");
    }
}


void clean(){
    printf("Cleaning\n");
    if(shared_memory != NULL) {
        shmdt(shared_memory);

    }
    if(shared_memory_id != -1) {
        shmctl(shared_memory_id, IPC_RMID, NULL);
    }

    if(sem_id != -1) {
        semctl(sem_id, 0, IPC_RMID);
    }

    for(int i=0; i<WORKER_1+WORKER_2+WORKER_3; i++){
        kill(pids[i], 9);
    }
}

void initialize_shared(){
    key_t s_memory_key = ftok(PATHNAME, PROJECT_ID)+10;

    shared_memory_id = shmget(s_memory_key, sizeof(shop_queue), IPC_CREAT | 0666);

    shared_memory = shmat(shared_memory_id, 0, 0);
    for(int i=0; i<ORDERS_LIMIT; i++) {
        shared_memory->queue[i].status = S_EMPTY;
        shared_memory->queue[i].size = -1;
    }
    shared_memory->last_send=-1;
    shared_memory->last_packed=-1;
    shared_memory->last_received=-1;
    shared_memory->num_packed=0;
    shared_memory->num_received=0;
    shmdt(shared_memory);

}

void initialize_sem(){
    key_t sem_key = ftok(PATHNAME, PROJECT_ID)+1;
    sem_id = semget(sem_key, 4, IPC_CREAT | 0666);
    if(sem_id != -1) semctl(sem_id, 0, IPC_RMID);
    sem_id = semget(sem_key, 4, IPC_CREAT | 0666);
    semctl(sem_id, SEM_FREE, SETVAL, ORDERS_LIMIT);
    semctl(sem_id, SEM_RECEIVED, SETVAL, 0);
    semctl(sem_id, SEM_PACKED, SETVAL, 0);
    semctl(sem_id, SEM_QUEUE_WRITE, SETVAL, 1);
}

int main(){

    atexit(clean);
    initialize_shared();
    initialize_sem();

    int current_pid=0;
    pid_t child;

    for(int i=0; i<WORKER_1; i++){
        child = fork();

        if(child == 0){
            execlp("./receive1", "receive1");
        }
        pids[current_pid++] = child;
    }

    for(int i=0; i<WORKER_2; i++){
        child = fork();

        if(child == 0){
            execlp("./pack2", "pack2");
        }
        pids[current_pid++] = child;
    }
    for(int i=0; i<WORKER_3; i++){
        child = fork();

        if(child == 0){
            execlp("./send3", "send3");
        }
        pids[current_pid++] = child;
    }
    while(1){
        usleep(10000);
    }
}
