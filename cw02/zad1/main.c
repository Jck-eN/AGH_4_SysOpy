#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/times.h>

clock_t start_time;
clock_t end_time;
struct tms start;
struct tms end;


int generate(char* file_path, int number_of_records, int record_length){
    FILE* file = fopen(file_path, "w+");
    if (file == NULL){
        printf("Error opening the file!");
        exit(1);
    }

    FILE* file_random = fopen("/dev/urandom", "r");
    if (file_random == NULL){
        printf("Error opening random file!");
        exit(1);
    }

    char* buff = malloc((record_length+1)*sizeof(char));
    for(int r_num=0; r_num < number_of_records; r_num++){
        if (fread(buff, sizeof(char), (size_t) record_length + 1, file_random) != record_length+1){
            printf("Error reading random data from buffer!");
            exit(1);
        }

        for (int j = 0; j < record_length; ++j) {
            buff[j] = (char)(abs(buff[j]) % 25 + 'a');
        }
        buff[record_length] =  '\n';

        if(fwrite(buff, sizeof(char), (size_t)record_length + 1, file) != record_length+1){
            printf("Error writing record data to file!");
            exit(1);
        };
    }

    fclose(file);
    fclose(file_random);
    free(buff);

    return 0;
}


void swap_records_lib(FILE* file, int record_number_1, int record_number_2, int record_length){
    if(record_number_1==record_number_2) return;
    char* rec_1 = calloc(record_length + 1, sizeof(char));
    char* rec_2 = calloc(record_length + 1, sizeof(char));
    fseek(file, record_number_1 * (record_length+1)*sizeof(char), 0);
    fread(rec_1, sizeof(char), record_length + 1, file);
    fseek(file, record_number_2 * (record_length+1)*sizeof(char), 0);
    fread(rec_2, sizeof(char), record_length + 1, file);

    fseek(file, record_number_2 * (record_length+1)*sizeof(char), 0);
    fwrite(rec_1, sizeof(char), record_length + 1, file);
    fseek(file, record_number_1 * (record_length+1)*sizeof(char), 0);
    fwrite(rec_2, sizeof(char), record_length + 1, file);
    free(rec_1);
    free(rec_2);
}

int is_second_lower_lib(FILE* file, int record_number_1, int record_number_2, int record_length){
    char* rec_1 = calloc(record_length + 1, sizeof(char));
    char* rec_2 = calloc(record_length + 1, sizeof(char));
    fseek(file, record_number_1 * (record_length+1)*sizeof(char), 0);
    fread(rec_1, sizeof(char), record_length + 1, file);
    fseek(file, record_number_2 * (record_length+1)*sizeof(char), 0);
    fread(rec_2, sizeof(char), record_length + 1, file);
    for(int i=0; i<record_length; i++){
        if(!rec_1[i] || !rec_2[i]) {
            printf("Problem while comapring records!");
            return 0;
        }
        if(rec_2[i]<rec_1[i]) {
            free(rec_1);
            free(rec_2);
            return 1;
        }

        else if(rec_2[i]>rec_1[i]) {
            free(rec_1);
            free(rec_2);
            return 0;
        }
    }
    free(rec_1);
    free(rec_2);
    return 0;
}

void sort_lib_internal(FILE* file, int first_record, int last_record, int record_length){
    if(first_record>= last_record) return;
    int pivot = first_record;
    int last_lower = pivot;
    for(int i=first_record+1; i<=last_record; i++){
        if(is_second_lower_lib(file, pivot, i, record_length)){
            last_lower++;
            swap_records_lib(file, i, last_lower, record_length);
        }
    }
    swap_records_lib(file, last_lower, pivot, record_length);
    sort_lib_internal(file, first_record, last_lower-1, record_length);
    sort_lib_internal(file, last_lower+1, last_record, record_length);
}

void sort_lib(char* file_path, int number_of_records, int record_length){
    FILE* file = fopen(file_path, "r+");
    if (file == NULL){
        printf("Error opening the file (sort_lib)!");
        exit(1);
    }
    sort_lib_internal(file, 0, number_of_records-1, record_length);

    fclose(file);
}

void swap_records_sys(int file, int record_number_1, int record_number_2, int record_length){
    if(record_number_1==record_number_2) return;
    char* rec_1 = calloc(record_length + 1, sizeof(char));
    char* rec_2 = calloc(record_length + 1, sizeof(char));
    lseek(file, record_number_1 * (record_length+1)*sizeof(char), SEEK_SET);
    read(file, rec_1, sizeof(char)* (record_length + 1));
    lseek(file, record_number_2 * (record_length+1)*sizeof(char), SEEK_SET);
    read(file, rec_2, sizeof(char)* (record_length + 1));

    lseek(file, record_number_2 * (record_length+1)*sizeof(char), SEEK_SET);
    write(file, rec_1, sizeof(char)* (record_length + 1));
    lseek(file, record_number_1 * (record_length+1)*sizeof(char), SEEK_SET);
    write(file, rec_2, sizeof(char)* (record_length + 1));

    free(rec_1);
    free(rec_2);
}

int is_second_lower_sys(int file, int record_number_1, int record_number_2, int record_length){
    char* rec_1 = calloc(record_length + 1, sizeof(char));
    char* rec_2 = calloc(record_length + 1, sizeof(char));
    lseek(file, record_number_1 * (record_length+1)*sizeof(char), SEEK_SET);
    read(file, rec_1, sizeof(char)* (record_length + 1));
    lseek(file, record_number_2 * (record_length+1)*sizeof(char), SEEK_SET);
    read(file, rec_2, sizeof(char)* (record_length + 1));

    for(int i=0; i<record_length; i++){
        if(!rec_1[i] || !rec_2[i]) {
            printf("Problem while comapring records!");
            return 0;
        }
        if(rec_2[i]<rec_1[i]) {
            free(rec_1);
            free(rec_2);
            return 1;
        }

        else if(rec_2[i]>rec_1[i]) {
            free(rec_1);
            free(rec_2);
            return 0;
        }
    }
    free(rec_1);
    free(rec_2);
    return 0;
}

void sort_sys_internal(int file, int first_record, int last_record, int record_length){
    if(first_record>= last_record) return;
    int pivot = first_record;
    int last_lower = pivot;
    for(int i=first_record+1; i<=last_record; i++){
        if(is_second_lower_sys(file, pivot, i, record_length)){
            last_lower++;
            swap_records_sys(file, i, last_lower, record_length);
        }
    }
    swap_records_sys(file, last_lower, pivot, record_length);
    sort_sys_internal(file, first_record, last_lower-1, record_length);
    sort_sys_internal(file, last_lower+1, last_record, record_length);
}

void sort_sys(char* file_path, int number_of_records, int record_length){
    int file = open(file_path, O_RDWR);
    if (file == -1){
        printf("Error opening the file (sort_sys)!");
        exit(1);
    }
    sort_sys_internal(file, 0, number_of_records-1, record_length);

    close(file);
}

void copy_lib(char* source, char* dest,  int number_of_records, int record_length){
    FILE* source_file = fopen(source, "r");
    FILE* dest_file = fopen(dest, "w+");

    char* buff = calloc(record_length, sizeof(char));
    for(int i=0; i<number_of_records; i++){
        if(fread(buff, sizeof(char), record_length+1, source_file) != record_length+1){
            printf("Error while reading file (copy_lib).");
            exit(1);
        }
        if(fwrite(buff, sizeof(char), record_length+1, dest_file) != record_length+1){
            printf("Error while writing to file (copy_lib).");
            exit(1);
        }
    }
    fclose(source_file);
    fclose(dest_file);
    free(buff);
}

void copy_sys(char* source, char* dest,  int number_of_records, int record_length){
    int source_file = open(source, O_RDONLY);
    int dest_file = open(dest, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    char* buff = calloc(record_length, sizeof(char));
    for(int i=0; i<number_of_records; i++){
        if(read(source_file, buff, sizeof(char)*(record_length+1)) != record_length+1){
            printf("Error while reading file (copy_sys).");
            exit(1);
        }
        if(write(dest_file, buff, sizeof(char)*(record_length+1)) != record_length+1){
            printf("Error while writing to file (copy_sys).");
            exit(1);
        }
    }
    close(source_file);
    close(dest_file);
    free(buff);
}

void start_timer() {
    start_time = times(&start);
}

void stop_timer(char* info_message, char* result_file_name) {
    end_time = times(&end);
    int64_t clk_tck = sysconf(_SC_CLK_TCK);

    char* message = calloc(strlen("User time: 000.0000\nSystem time: 000.0000\n") + 1, sizeof(char*));

    double user_time = (double)(end.tms_utime - start.tms_utime) / clk_tck;
    double system_time = (double)(end.tms_stime - start.tms_stime) / clk_tck;

    sprintf(message, "User time: %.2fs\nSystem time: %.2fs\nAll time: %.2fs\n", user_time, system_time, user_time+system_time);

    printf("%s", info_message);
    printf("%s", message);

    FILE* fp = fopen(result_file_name, "a");
    fputs(info_message, fp);
    fputs(message, fp);
    fclose(fp);
    free(message);
}

int main(int argc, char const *argv[])
{
    int i=1;
    char info_message[256];
    while(i < argc){
        if(strcmp(argv[i], "generate") == 0 && argc > i+3) {
            sprintf(info_message, "Time of GENERATING measured for %d blocks and record length %d \n", atoi(argv[i + 2]), atoi(argv[i + 3]));
            start_timer();
            generate(argv[i+1], atoi(argv[i+2]), atoi(argv[i+3]));
            stop_timer(info_message, "wyniki.txt");
            i+=4;
        }
        else if(strcmp(argv[i], "sort") == 0 && argc > i+4) {
            if(strcmp(argv[i+4], "sys") == 0 ) {
                sprintf(info_message, "Time of SORTING (SYS), measured for %d blocks and record length %d \n", atoi(argv[i + 2]), atoi(argv[i + 3]));
                start_timer();
                sort_sys(argv[i + 1], atoi(argv[i + 2]), atoi(argv[i + 3]));
                stop_timer(info_message, "wyniki.txt");
            }
            else{
                sprintf(info_message, "Time of SORTING (LIB), measured for %d blocks and record length %d \n", atoi(argv[i + 2]), atoi(argv[i + 3]));
                start_timer();
                sort_lib(argv[i+1], atoi(argv[i+2]), atoi(argv[i+3]));
                stop_timer(info_message, "wyniki.txt");
            }
            i+=5;
        }
        else if(strcmp(argv[i], "copy") == 0 && argc > i+5) {
            if(strcmp(argv[i+5], "sys") == 0 ){
                sprintf(info_message, "Time of COPYING (SYS), measured for %d blocks and record length %d \n", atoi(argv[i + 3]), atoi(argv[i + 4]));
                start_timer();
                copy_sys(argv[i+1], argv[i+2], atoi(argv[i+3]), atoi(argv[i+4]));
                stop_timer(info_message, "wyniki.txt");
            }
            else{
                sprintf(info_message, "Time of COPYING (LIB), measured for %d blocks and record length %d \n", atoi(argv[i + 3]), atoi(argv[i + 4]));
                start_timer();
                copy_lib(argv[i+1], argv[i+2], atoi(argv[i+3]), atoi(argv[i+4]));
                stop_timer(info_message, "wyniki.txt");
            }
            i+=6;
        }
    }
    return 0;
}
