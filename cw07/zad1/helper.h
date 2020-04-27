#ifndef HELPER_H
#define HELPER_H

#define ORDERS_LIMIT 10
#define MAX_PACKAGE_SIZE 20

#define PATHNAME "/home/"
#define PROJECT_ID 'J'

#define order_status int

#define S_EMPTY 0
#define S_RECEIVED 1
#define S_PACKED 2

typedef struct order{
    int size;
    order_status status;
} order;


typedef struct shop_queue{
    order queue[ORDERS_LIMIT];
    int last_received;
    int num_received;
    int last_packed;
    int num_packed;
    int last_send;

} shop_queue;


#define SEM_QUEUE_WRITE 0
#define SEM_FREE 1
#define SEM_RECEIVED 2
#define SEM_PACKED 3


#define WORKER_1 2
#define WORKER_2 2
#define WORKER_3 2

#endif //HELPER_H
