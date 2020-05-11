#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define MIN_TIME_RETRY 3
#define MAX_TIME_RETRY 5

#define MIN_TIME_SPAWN 1
#define MAX_TIME_SPAWN 2

#define MIN_TIME_SHAVE 2
#define MAX_TIME_SHAVE 4

#define CHAIR_FREE 0

pthread_mutex_t clients_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t client_in_queue = PTHREAD_COND_INITIALIZER;

int barber_in_work = 0;
int clients_num;

struct clients_queue {
    int* chair_status;
    int max_size;
    int engaged_places;
} clients_queue;


int queue_get_free_place(){
    for(int i=0; i<clients_queue.max_size; i++){
        if(clients_queue.chair_status[i] == CHAIR_FREE){
            return i;
        }
    }
    return -1;
}

int queue_get_client_place(){
    int first_client = -1;
    for(int i=0; i<clients_queue.max_size; i++){
        if(clients_queue.chair_status[i] != CHAIR_FREE){
            first_client = i;
        }
    }
    if(first_client == -1) return -1;
    for(int i=0; i<clients_queue.max_size; i++){
        if(clients_queue.chair_status[i] != CHAIR_FREE
        && clients_queue.chair_status[i] < clients_queue.chair_status[first_client]){
            first_client = i;
        }
    }
    return first_client;
}

int get_random_time_ms(int min_ms, int max_ms){
    return min_ms + (rand() % (max_ms - min_ms + 1));
}

void client_approach(int thread_id){

    pthread_mutex_lock(&clients_queue_mutex);

    int free_place = queue_get_free_place();
    if(free_place == -1){
        pthread_mutex_unlock(&clients_queue_mutex);
        printf("Zajęte; %d\n", thread_id);

        usleep(1000 * get_random_time_ms(MIN_TIME_RETRY * 1000, MAX_TIME_RETRY * 1000));

        client_approach(thread_id);
        return;
    }

    clients_queue.chair_status[free_place] = thread_id;
    clients_queue.engaged_places++;

    if(clients_queue.engaged_places > 0 && barber_in_work == 0){
        printf("Budzę golibrodę; %d\n", thread_id);
        pthread_cond_signal(&client_in_queue);
    }
    else{
        printf("Poczekalnia, wolne miejsca: %d; %d\n", clients_queue.max_size - clients_queue.engaged_places, thread_id);
    }

    pthread_mutex_unlock(&clients_queue_mutex);
}


void barber() {
    int customers_shaved = 0;
    while(clients_num > customers_shaved){

        pthread_mutex_lock(&clients_queue_mutex);

        while(clients_queue.engaged_places == 0){
            barber_in_work = 0;
            printf("Golibroda: idę spać...\n");
            pthread_cond_wait(&client_in_queue, &clients_queue_mutex);
        }
        barber_in_work = 1;
        int client_place = queue_get_client_place();
        if(client_place != -1){
            clients_queue.engaged_places--;
            customers_shaved++;
            printf("Golibroda: czeka %d klientów, golę klienta: %d\n",
                    clients_queue.engaged_places,
                    clients_queue.chair_status[client_place]);
            clients_queue.chair_status[client_place] = CHAIR_FREE;
        }

        pthread_mutex_unlock(&clients_queue_mutex);

        usleep(get_random_time_ms(MIN_TIME_SHAVE * 1000, MAX_TIME_SHAVE * 1000) * 1000);
    }
}

int main(int argc, char **argv){
    if (argc != 3) {
        printf("Nieprawidłowe wywołanie programu:\n");
        printf("---\n");
        printf("Użycie:\n");
        printf("main <chairs_number> <clients_number>\n");
    }
    srand(time(NULL)+getpid());

    int chairs_num = atoi(argv[1]);
    clients_num = atoi(argv[2]);

    clients_queue.engaged_places = 0;
    clients_queue.max_size = chairs_num;
    clients_queue.chair_status = calloc(chairs_num, sizeof(int));

    pthread_t barber_thread;

    pthread_t threads[clients_num];
    pthread_t thread_ids[clients_num];


    pthread_create(&barber_thread, NULL, (void *(*)(void *)) barber, NULL);

    for(int i=0; i<clients_num; i++){
        thread_ids[i] = i+1;
        usleep(1000 * get_random_time_ms(MIN_TIME_SPAWN * 1000, MAX_TIME_SPAWN * 1000));
        pthread_create(&threads[i], NULL, (void *(*)(void *)) client_approach, (void* ) thread_ids[i]);
    }

    pthread_join(barber_thread, NULL);

    for(int i=0; i<clients_num; i++){
        pthread_join(threads[i], NULL);
    }

    free(clients_queue.chair_status);

    return 0;
}