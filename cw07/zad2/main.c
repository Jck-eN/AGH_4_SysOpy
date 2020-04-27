#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "helper.h"
#include <unistd.h>
#include <semaphore.h>

pid_t pid = -1;
char current_time[100];
int shared_memory_id = -1;
shop_queue* shared_memory = NULL;
int sem_id = -1;
pid_t pids[WORKER_1+WORKER_2+WORKER_3];
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


void clean(){
    shm_unlink("/shared");

    for (int i = 0; i < 4; i++) {
        if (sem_unlink(sem_names[i]) < 0) {
            printf("Cannot unline semaphore\n");
        }
    }

    for(int i=0; i<WORKER_1+WORKER_2+WORKER_3; i++){
        kill(pids[i], 9);
    }
}

void initialize_shared(){
    shared_memory_id = shm_open(SHARED_MEM, O_RDWR | O_CREAT, 0666);
    if(shared_memory_id < 0){
        printf("Cannot create shared memory");
        exit(1);
    }
    if(ftruncate(shared_memory_id, sizeof(shop_queue)) <0){
        printf("Cannot truncate shared_memory!");
        exit(1);
    }
    shared_memory = mmap(NULL, sizeof(shop_queue), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
    for(int i=0; i<ORDERS_LIMIT; i++) {
        shared_memory->queue[i].status = S_EMPTY;
        shared_memory->queue[i].size = -1;
    }
    shared_memory->last_send=-1;
    shared_memory->last_packed=-1;
    shared_memory->last_received=-1;
    shared_memory->num_packed=0;
    shared_memory->num_received=0;
    if (munmap(shared_memory, sizeof(shop_queue)) < 0)
    {
        printf("Cannot detach shared memory\n");
        printf("Errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    shared_memory = mmap(NULL, sizeof(shop_queue), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
    munmap(shared_memory, sizeof(shop_queue));
}

void initialize_sem(){
    for(int i=0; i<4; i++){
        sem_unlink(sem_names[i]);
    }
    semaphores[0] = sem_open(sem_names[0], O_RDWR | O_CREAT, 0666, 1);
    semaphores[1] = sem_open(sem_names[1], O_RDWR | O_CREAT, 0666, ORDERS_LIMIT);

    for(int i=2; i<4;i++){
        semaphores[i] = sem_open(sem_names[i], O_RDWR | O_CREAT, 0666, 0);
    }

    for(int i=0; i<4;i++){
        sem_close(semaphores[i]);
    }
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
            fflush(stdout);
            execlp("./receive1", "receive1");
        }
        pids[current_pid++] = child;
    }

    for(int i=0; i<WORKER_2; i++){
        child = fork();

        if(child == 0){
            fflush(stdout);
            execlp("./pack2", "pack2");
        }
        pids[current_pid++] = child;
    }
    for(int i=0; i<WORKER_3; i++){
        child = fork();

        if(child == 0){
            fflush(stdout);
            execlp("./send3", "send3");
        }
        pids[current_pid++] = child;
    }
    while(1){
        usleep(10000);
    }
}
