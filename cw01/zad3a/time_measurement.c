#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib_diff.h"
#include <sys/times.h>
#include <time.h>

void remove_blocks(struct main_arr* array, int number){
    for(int i=0; i< number; i++){
        remove_block(array, i);
    }
}

void add_and_remove_blocks(struct main_arr* array, int number){
    for(int i = 0;i<number; i++){
        add_block(array, "tmp");
        remove_block(array, number+i);
    }
}

double calculate_time(clock_t start, clock_t end){
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}


int main(int argc, char const *argv[])
{
    printf("Data: %s\n", argv[1]);

    int num_of_pairs = argc - 2;
    struct pair_sequence* seq = create_files_sequence(num_of_pairs, argv+2);
    struct main_table* table = create_main_arr(2*num_of_pairs + 2);

    struct tms **tms_time = calloc(6, sizeof(struct tms *));
    clock_t real_time[6];
    for (int i = 0; i < 6; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms));
    }

    real_time[0] = times(tms_time[0]);
    compare_sequence(table, seq);
    real_time[1] = times(tms_time[1]);

    int block_s =  block_size(table, 0);

    real_time[2] = times(tms_time[2]);
    remove_blocks(table, num_of_pairs);
    real_time[3] = times(tms_time[3]);

    real_time[4] = times(tms_time[4]);
    add_and_remove_blocks(table, num_of_pairs);
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
