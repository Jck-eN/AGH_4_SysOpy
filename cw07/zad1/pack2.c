#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "helper.h"

pid_t pid = -1;
char current_time[100];
int shared_memory_id = -1;
shop_queue* shared_memory = NULL;
int sem_id = -1;

void change_sem(int sem_num, int sem_diff){
    if(sem_id != -1){
        struct sembuf sem_request;
        sem_request.sem_num = sem_num;
        sem_request.sem_op = sem_diff;
        sem_request.sem_flg = 0;

        if (semop(sem_id, &sem_request, 1))
            printf("Nie udało się zmienić wartości semafora!\n");
    }
}

void get_current_time(char* time_string){
    time_t now = time(NULL);
    struct tm *now_time = localtime(&now);
    char buf[30];
    strftime(buf, 29, "%H:%M:%S", now_time);
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int ms = tv.tv_usec/1000;
    sprintf(time_string, "%s.%d", buf, ms);
}

void pack_order(){
    int to_pack = -1;
    int to_send = -1;
    int new_size =-1;

    change_sem(SEM_RECEIVED, -1);
    shared_memory = shmat(shared_memory_id, NULL, 0);
    if(shared_memory == NULL) return;

    int package_to_pack = (shared_memory->last_packed+1)%ORDERS_LIMIT;



    if(shared_memory->queue[package_to_pack].status == S_RECEIVED){

        change_sem(SEM_QUEUE_WRITE, -1);

        new_size = shared_memory->queue[package_to_pack].size*2;
        shared_memory->queue[package_to_pack].size = new_size;
        shared_memory->queue[package_to_pack].status = S_PACKED;
        shared_memory->last_packed = package_to_pack;
        shared_memory->num_received--;
        shared_memory->num_packed++;
        to_pack = shared_memory->num_received;
        to_send = shared_memory->num_packed;

        change_sem(SEM_PACKED, 1);

        change_sem(SEM_QUEUE_WRITE, 1);
    }
    else{
        printf("Error queue status (pack2)\n");

        shmdt(shared_memory);
        return;
    }

    get_current_time(current_time);
    printf("(%d %s) Przygotowałem zamówienie o wielkości %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n",
           pid, current_time, new_size, to_pack, to_send);

    shmdt(shared_memory);
}

void clean(){
    if(shared_memory != NULL) {
        shmdt(shared_memory);
    }
}

void initialize_shared(){
    key_t s_memory_key = ftok(PATHNAME, PROJECT_ID)+10;
    shared_memory_id = shmget(s_memory_key, sizeof(shop_queue), IPC_CREAT);
}

void initialize_sem(){
    key_t sem_key = ftok(PATHNAME, PROJECT_ID)+1;
    sem_id = semget(sem_key, 0, 0);
}


int main() {
    atexit(clean);
    int package_size=0;
    int sleep_us = 0;
    pid = getpid();
    srand(time(NULL)+5+getpid());
    initialize_shared();
    initialize_sem();


    while(1){
        package_size = rand()%MAX_PACKAGE_SIZE+1;

        pack_order();

        sleep_us = rand()%1000000+1000000;
        usleep(sleep_us);
    }


    return 0;
}
