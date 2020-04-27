#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "helper.h"
#include <sys/stat.h>
#include <semaphore.h>

pid_t pid = -1;
char current_time[100];
int shared_memory_id = -1;
shop_queue* shared_memory = NULL;
int sem_id = -1;
sem_t* semaphores[4];

void change_sem(int sem_num, int sem_diff){
    if(sem_num>=4 || sem_num < 0) return;
    if(semaphores[sem_num] == NULL ) return;

    if(sem_diff == 1){
        sem_post(semaphores[sem_num]);
    }
    else if(sem_diff == -1){
        sem_wait(semaphores[sem_num]);
    }
    else return;

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

void send_order(){

    change_sem(SEM_PACKED, -1);
    shared_memory = mmap(NULL, sizeof(shop_queue), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
    int to_pack = -1;
    int to_send = -1;
    int send_size = -1;
    if(shared_memory == NULL) return;

    int package_to_send = (shared_memory->last_send + 1) % ORDERS_LIMIT;


    if(shared_memory->queue[package_to_send].status == S_PACKED){

        change_sem(SEM_QUEUE_WRITE, -1);

        send_size = shared_memory->queue[package_to_send].size * 3;
        shared_memory->queue[package_to_send].size = -1;
        shared_memory->queue[package_to_send].status = S_EMPTY;
        shared_memory->last_send = package_to_send;
        shared_memory->num_packed--;
        to_pack = shared_memory->num_received;
        to_send = shared_memory->num_packed;

        change_sem(SEM_FREE, 1);

        change_sem(SEM_QUEUE_WRITE, 1);

    }
    else{
        printf("Error queue status (send3)\n");

        munmap(shared_memory, sizeof(shop_queue));
        return;
    }

    get_current_time(current_time);
    printf("(%d %s) Wysłałem zamówienie o wielkości %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n",
           pid, current_time, send_size, to_pack, to_send);

    munmap(shared_memory, sizeof(shop_queue));
}

void clean(){
    if(shared_memory != NULL) {
        if (munmap(shared_memory, sizeof(shop_queue)) < 0)
        {
            printf("Cannot detach shared memory\n");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (sem_close(semaphores[i]) < 0)
        {
            printf("Cannot close semaphore\n");
            exit(EXIT_FAILURE);
        }
    }

}

void initialize_shared(){
    shared_memory_id = shm_open(SHARED_MEM, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO );
    if(shared_memory_id < 0){
        printf("Cannot create shared memory");
        exit(1);
    }
}


void initialize_sem(){
    for (int i = 0; i < 4; i++)
    {
        semaphores[i] = sem_open(sem_names[i], O_RDWR);
        if (semaphores[i] < 0)
        {
            printf("Cannot open semaphore\n");
            exit(1);
        }
    }
}

int main() {
    atexit(clean);
    int package_size=0;
    int sleep_us = 0;
    pid = getpid();
    srand(time(NULL)+7+getpid());
    initialize_shared();
    initialize_sem();


    while(1){
        package_size = rand()%MAX_PACKAGE_SIZE+1;

        send_order();

        sleep_us = rand()%1000000+2000000;
        usleep(sleep_us);
    }

    return 0;
}
