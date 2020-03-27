#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib_diff.h"
#include <sys/times.h>
#include <time.h>
#include <dlfcn.h>



double calculate_time(clock_t start, clock_t end){
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}


int main(int argc, char const *argv[])
{
    void* dl_handle = dlopen("./lib_diff.so", RTLD_LAZY);
    if(!dl_handle) {
        printf("Cannot load dynamic library %s\n", dlerror());
        return 1;
    }

    struct main_arr* (*dl_create_main_arr)(int);
    dl_create_main_arr = (struct main_arr* (*)(int))dlsym(dl_handle, "create_main_arr");

    struct pairs_sequence* (*dl_create_files_sequence)(int, char**);
    dl_create_files_sequence = (struct pairs_sequence* (*)(int, char**))dlsym(dl_handle, "create_files_sequence");

    void (*dl_compare_sequence)(struct main_arr*, struct pairs_sequence*);
    dl_compare_sequence = (void (*)(struct main_arr*, struct pairs_sequence*))dlsym(dl_handle, "compare_sequence");

    int (*dl_add_block)(struct main_arr*, char*);
    dl_add_block = (int (*)(struct main_arr*, char*))dlsym(dl_handle, "add_block");

    int (*dl_block_size)(struct main_arr*, int);
    dl_block_size = (int (*)(struct main_arr*, int))dlsym(dl_handle, "block_size");

    void (*dl_remove_operation)(struct operations_block* , int);
    dl_remove_operation = (void (*)(struct operations_block*,  int))dlsym(dl_handle, "remove_operation");

    void (*dl_remove_block)(struct main_arr*, int);
    dl_remove_block = (void (*)(struct main_arr*, int))dlsym(dl_handle, "remove_block");

    
    printf("Data: %s\n", argv[1]);

    int num_of_pairs = argc - 2;
    struct pair_sequence* seq = dl_create_files_sequence(num_of_pairs, argv+2);
    struct main_table* table = dl_create_main_arr(2*num_of_pairs + 2);

    struct tms **tms_time = calloc(6, sizeof(struct tms *));
    clock_t real_time[6];
    for (int i = 0; i < 6; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms));
    }

    real_time[0] = times(tms_time[0]);
    dl_compare_sequence(table, seq);
    real_time[1] = times(tms_time[1]);

    int block_s =  dl_block_size(table, 0);

    real_time[2] = times(tms_time[2]);
    for(int i=0; i< num_of_pairs; i++){
        dl_remove_block(table, i);
    }
    real_time[3] = times(tms_time[3]);

    real_time[4] = times(tms_time[4]);    
    for(int i = 0;i<num_of_pairs; i++){
        dl_add_block(table, "tmp");
        dl_remove_block(table, num_of_pairs+i);
    }
    real_time[5] = times(tms_time[5]);


    printf("For block size: %zu\n", block_s);

    printf("REAL TIME    User    System\n");

    printf("compare %d pairs\n", num_of_pairs);
    printf("%lf   ", calculate_time(real_time[0], real_time[1]));
    printf("%lf   ", calculate_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime));
    printf("%lf   \n", calculate_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime));

    printf("delete block avg\n");
    printf("%lf   ", calculate_time(real_time[2], real_time[3]) / num_of_pairs);
    printf("%lf   ", calculate_time(tms_time[2]->tms_utime, tms_time[3]->tms_utime) / num_of_pairs );
    printf("%lf   \n", calculate_time(tms_time[2]->tms_stime, tms_time[3]->tms_stime) / num_of_pairs);


    printf("add and delete avg\n");
    printf("%lf   ", calculate_time(real_time[4], real_time[5]) / num_of_pairs);
    printf("%lf   ", calculate_time(tms_time[4]->tms_utime, tms_time[5]->tms_utime) / num_of_pairs );
    printf("%lf   \n", calculate_time(tms_time[4]->tms_stime, tms_time[5]->tms_stime) / num_of_pairs);
    printf("\n\n");
    return 0;
}
