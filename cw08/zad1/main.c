#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdatomic.h>
#include <pthread.h>
#include <math.h>


#define MAX_COLORS 256
#define MAX_LINE_LENGTH 128
const int RES_HEIGHT = 300;
const int RES_WIDTH_MULT = 5;

int threads_number;
int** image;
int width;
int height;
int mode;
int histogram[MAX_COLORS] = {0};

#define MAX_COLORS 256

long get_time(struct timespec *start) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    long usec_time = (end.tv_sec - start->tv_sec) * 1000000;
    usec_time += (end.tv_nsec - start->tv_nsec) / 1000.0;
    return usec_time;
}

void set_mode(char* mode_str){
    if(strcmp(mode_str, "sign") == 0){
        mode = 0;
    }
    else if(strcmp(mode_str, "block") == 0){
        mode = 1;
    }
    else if(strcmp(mode_str, "interleaved") == 0){
        mode = 2;
    }
    else {
        perror("Wrong mode");
        exit(EXIT_FAILURE);
    }
    return;
}

void load_image(char* input_filename){
    FILE* input_file = fopen(input_filename, "r");
    if(input_file == NULL) {
        perror("Nie można otworzyć pliku z obrazem!\n");
        exit(EXIT_FAILURE);
    }
    char buff[MAX_LINE_LENGTH+1];
    fgets(buff, MAX_LINE_LENGTH, input_file);
    do{
        fgets(buff, MAX_LINE_LENGTH, input_file);
    } while(buff[0] == '#');

    char* width_str;
    char* height_str;
    char* content = buff;

    width_str = strtok_r(content, " \t\r\n", &content);
    height_str = strtok_r(NULL, " \t\r\n", &content);
    width = atoi(width_str);
    height = atoi(height_str);
    image = calloc(height, sizeof(int* ));
    for(int i=0; i<height; i++){
        image[i] = calloc(width, sizeof(int));
    }

    char* line_buff = NULL;
    int len = 0;

    fgets(buff, MAX_LINE_LENGTH, input_file);

    for(int row=0; row < height; row++){
        getline(&line_buff, &len, input_file);

        char *number;
        char *remainder = line_buff;
        for (int col = 0; col < width; col++)
        {
            number = strtok_r(remainder, " \t\r\n", &remainder);
            if(number == NULL){
                getline(&line_buff, &len, input_file);
                remainder = line_buff;
                number = strtok_r(remainder, " \t\r\n", &remainder);
            }
            image[row][col] = atoi(number);
        }
    }
    fclose(input_file);
}

void save_histogram(const char *out_filename){

    const int RES_WIDTH = MAX_COLORS * RES_WIDTH_MULT;

    FILE *out_file = fopen(out_filename, "w+");
    if(out_file == NULL){
        printf("Cannot open file %s for write", out_filename);
        exit(EXIT_FAILURE);
    }

    int max = histogram[0];
    for(int i=1; i< MAX_COLORS; i++){
        if(histogram[i] > max){
            max = histogram[i];
        }
    }
    fprintf(out_file, "P2\n");
    fprintf(out_file, "%d %d\n", RES_WIDTH, RES_HEIGHT);
    fprintf(out_file, "%d\n", MAX_COLORS-1);


    double max_d = max;
    for(int row = 0; row < RES_HEIGHT; row++){
        for(int i=0; i<MAX_COLORS; i++){
            if(RES_HEIGHT - row < RES_HEIGHT * (histogram[i] / max_d)){
                for(int j=0; j<RES_WIDTH_MULT; j++){
                    fprintf(out_file, "%d ", MAX_COLORS-1);
                }
            }
            else{
                for(int j=0; j<RES_WIDTH_MULT; j++){
                    fprintf(out_file, "0 ");
                }
            }
        }
        fputs("\n", out_file);
    }

    fclose(out_file);
}

long sign_mode(int* thread_id){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int start_number;
    int end;
    int current = *thread_id;
    int equally_devided = threads_number - (MAX_COLORS % threads_number);
    int per_thread = MAX_COLORS / threads_number;

    if(MAX_COLORS % threads_number == 0 || current < equally_devided){
        start_number = current * per_thread;
        end = (current + 1) * per_thread;
    }else{
        start_number = (equally_devided * per_thread) + (current - equally_devided) * (per_thread + 1);
        end = start_number + per_thread + 1;
    }

    int color;
    for(int row = 0; row < height; row++){
        for(int col=0; col < width; col++) {
            color = image[row][col];
            if (start_number <= color && color < end) {
                atomic_fetch_add(&histogram[color], 1);
            }
        }
    }

    return get_time(&start);
}

long block_mode(int* thread_id){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int k = *thread_id;
    double tmp1 = ((double)width)/threads_number;
    double tmp = ceil(width/threads_number);
    int start_col = k*tmp;
    int end_col = (k+1)*tmp;

    int color;
    for(int col=start_col; col < end_col; col++) {
        for(int row = 0; row < height; row++){
            color = image[row][col];
            atomic_fetch_add(&histogram[color], 1);
        }
    }

    return get_time(&start);
}

long interleaved_mode(int* thread_id){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int k = *thread_id;
    int color;
    for(int col=k; col<width; col+=threads_number){
        for(int row = 0; row < height; row++){
            color = image[row][col];
            atomic_fetch_add(&histogram[color], 1);
        }
    }

    return get_time(&start);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Nieprawidłowe wywołanie programu:\n");
        printf("---\n");
        printf("Usage:\n");
        printf("main <threads_number> <sign|mode|interleaved> <input_file> <output_file>\n");
    }

    threads_number = atoi(argv[1]);
    char *mode_str = argv[2];
    set_mode(mode_str);
    char *input_filename = argv[3];
    char *output_file = argv[4];
    load_image(input_filename);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    long (*calculate_function[3])(int *) = {
            sign_mode,
            block_mode,
            interleaved_mode
    };


    pthread_t *threads = calloc(threads_number, sizeof(pthread_t));
    int *thread_ids = calloc(threads_number, sizeof(int));
    for(int i=0; i<threads_number; i++){
        thread_ids[i]=i;
        pthread_create(&threads[i], NULL, (void* (*)(void*)) calculate_function[mode], (void*)(&thread_ids[i]));
    }

    long result;
    for(int i=0; i<threads_number; i++){
        pthread_join(threads[i], (void *) &result);
        printf("Wątek %d:\t\t %ld ms\n", thread_ids[i], result);
    }

    printf("Cały program:\t %ld ms\n", get_time(&start));

    save_histogram(output_file);

    for (int i = 0; i < height; i++) {
        free(image[i]);
    }
    free(image);
    free(threads);
    free(thread_ids);
}
