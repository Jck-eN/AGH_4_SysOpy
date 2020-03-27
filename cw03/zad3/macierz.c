#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <linux/limits.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>

typedef struct
{
    int **data;
    int rows;
    int cols;
} matrix;


int count_rows(FILE* file){
    fseek(file, 0, SEEK_SET);
    int rows = 0;
    for (char c = getc(file); c != EOF; c = getc(file))
        if (c == '\n')
            rows++;
    return rows;
}

int count_columns(FILE* file){
    int cols=0;
    fseek(file, 0, SEEK_SET);
    for (char c = getc(file); c != '\n' && c != EOF; c = getc(file))
        if (c == ' ' && (c+1) !='\n')
            cols++;
    return cols+1;
}

matrix* read_matrix(char* filename){
    FILE* input = fopen(filename, "r");
    if(input == NULL){
        printf("Cannot load file! (read_matrix)");
        exit(1);
    }
    fseek(input, 0, SEEK_SET);
    matrix* result = calloc(1, sizeof(matrix));

    result->rows = count_rows(input);
    result->cols = count_columns(input);

    fseek(input, 0, SEEK_SET);
    result->data = calloc(result->rows, sizeof(int*));
    for(int i=0; i< result->rows; i++){
        result->data[i] = calloc(result->cols, sizeof(int));
        for(int j=0; j< result->cols; j++){
            fscanf(input, "%d", &result->data[i][j]);
        }
    }
    fclose(input);
    return result;
}

void multiply_column_to_tmp(matrix *first, matrix *second, int col_number, int pair_number)
{
    int col_idx = col_number-1;
    if(col_number <= 0){
        printf("multiply_column_to_tmp -> col_number <=0\n");
        return;
    }
    char *output_filename = calloc(32, sizeof(char));
    sprintf(output_filename, "tmp_m%03d_part%03d", pair_number, col_number);
    FILE *output = fopen(output_filename, "w+");
    for (int row = 0; row < first->rows; row++){
        int result = 0;
        for (int col = 0; col < first->cols; col++){
            result += first->data[row][col] * second->data[col][col_idx];
        }
        fprintf(output, "%09d\n", result);
    }
    free(output_filename);
    fclose(output);
}
void free_matrix(matrix* m)
{
    for (int y = 0; y < m->rows; y++){
        free(m->data[y]);
    }
    free(m->data);
    free(m);
}

void write_number_to_file(FILE* output, int col_idx, int row, int all_columns, int number){
    int fd = fileno(output);
    flock(fd, LOCK_EX);
    fseek(output, row*(10*all_columns)+col_idx*(10), SEEK_SET);
    fprintf(output, "%09d", number);
    flock(fd, LOCK_UN);

}

void multiply_column_file(matrix *first, matrix *second, int col_number, char* output_file)
{
    int col_idx = col_number-1;
    if(col_number <= 0){
        printf("multiply_column_file -> col_number <=0\n");
        return;
    }
    FILE* output = fopen(output_file, "r+");

    for (int row = 0; row < first->rows; row++){
        int result = 0;
        for (int col = 0; col < first->cols; col++){
            result += first->data[row][col] * second->data[col][col_idx];
        }
        write_number_to_file(output, col_idx, row, second->cols, result);
    }
    fclose(output);

}



//Number can be from 1 to 3 (input1, input2, output)
char* read_matrix_filename(char* list_file, int matrix_pair , int number){
    FILE* list = fopen(list_file, "r");
    if(list == NULL) return NULL;
    fseek(list, 0, SEEK_SET);
    char* buff = calloc(1, 100*sizeof(char));
    char* source1 = calloc(1, 20* sizeof(char));
    char* source2 = calloc(1, 20* sizeof(char));
    char* dest = calloc(1, 20* sizeof(char));
    int row=1;
    for (char c='\0'; c != EOF; c = getc(list)){
        if (c == '\n'){
            row++;
        }
        if(row == matrix_pair){
            fscanf(list, "%s %s %s", source1, source2, dest);
            break;
        }
    }

    if(number == 1){
        free(buff);
        free(source2);
        free(dest);
        fclose(list);
        return source1;
    }
    else if(number == 2){
        free(buff);
        free(source1);
        free(dest);
        fclose(list);
        return source2;
    }
    else if(number == 3){
        free(buff);
        free(source1);
        free(source2);
        fclose(list);
        return dest;
    }
    free(buff);
    free(source1);
    free(source2);
    free(dest);
    fclose(list);
    return NULL;
}

void child_process(int process_number, int* columns_number, int* first_columns,
        int max_time, int mode, int pairs, char* list,
        int cpu_limit, int mem_limit){
    int multiplications = 0;
    time_t start_time = time(NULL);
   struct rlimit cpu, mem, limit;
   cpu.rlim_max = cpu_limit;
   cpu.rlim_cur = cpu_limit;
   mem.rlim_max = 1000000*mem_limit;
   mem.rlim_cur = 1000000*mem_limit;

   setrlimit(RLIMIT_CPU, &cpu);
   setrlimit(RLIMIT_AS,  &mem);

    for(int pair = 1; pair <= pairs; pair++)
    {
        if(time(NULL) - start_time > max_time) {
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            printf("Proces o PID %d: CPU - system : %fs, user : %fs, Mem usage - %dkb\n",
                   getpid(),
                   ((float)usage.ru_stime.tv_usec)/1000000+(float)usage.ru_stime.tv_sec,
                   ((float)usage.ru_utime.tv_usec)/1000000+(float)usage.ru_utime.tv_sec,
                   usage.ru_maxrss);

            exit(multiplications);
        }

        int n_cols = columns_number[pair-1];
        int first_col = first_columns[pair-1];
        char* matrix1 = read_matrix_filename(list, pair, 1);
        char* matrix2 = read_matrix_filename(list, pair, 2);

        matrix* m1 = read_matrix(matrix1);
        matrix* m2 = read_matrix(matrix2);
        for(int col=first_col; col<first_col+n_cols; col++)
        {
            if(time(NULL) - start_time > max_time) {
                struct rusage usage;
                printf("AProces o PID %d: CPU - system : %fs, user : %fs, Mem usage - %dkb\n",
                       getpid(),
                       ((float)usage.ru_stime.tv_usec)/1000000+(float)usage.ru_stime.tv_sec,
                       ((float)usage.ru_utime.tv_usec)/1000000+(float)usage.ru_utime.tv_sec,
                       usage.ru_maxrss);

                exit(multiplications);
            }
            if(mode == 0){
                multiply_column_to_tmp(m1, m2, col, pair);
            }
            else{
                char* dest = read_matrix_filename(list, pair, 3);
                multiply_column_file(m1, m2, col, dest);
            }
//            sleep(1);
            multiplications++;
        }
        free_matrix(m1);
        free_matrix(m2);
        free(matrix1);
        free(matrix2);
    }
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Proces o PID %d: CPU - system : %fs, user : %fs, Mem usage - %dkb\n",
            getpid(),
           ((float)usage.ru_stime.tv_usec)/1000000+(float)usage.ru_stime.tv_sec,
           ((float)usage.ru_utime.tv_usec)/1000000+(float)usage.ru_utime.tv_sec,
           usage.ru_maxrss);

    exit(multiplications);
}

int main(int argc, char* argv[]){
    if(argc < 7){
        printf("Not enought arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: macierz <list> <num_of_processes> <max_time> <type> <hard_limit_cpu> <hard_limit_mem>\n");
        printf("list - file with list of matrices \n");
        printf("num_of_processes - number of child processes used\n");
        printf("max_time - maximum time for process execution [s] \n");
        printf("type - com|sep - one common output file or multiple separate files \n");
        printf("hard_limit_cpu - hard limit for child process cpu usage [s] \n");
        printf("hard_limit_mem - hard limit for child process memory usage [MB]  \n");
        return 0;
    }



//    struct rlimit limit;
//    getrlimit(RLIMIT_CPU, &limit);
//    printf("Proces %d: CPU - hard: %d, CPU - soft: %d\n", getpid(), limit.rlim_max, limit.rlim_cur);
//    getrlimit(RLIMIT_AS, &limit);
//    printf("Proces %d: MEM - hard: %d, MEM - soft: %d\n", getpid(), limit.rlim_max, limit.rlim_cur);
//
//    struct rlimit cpu, mem;
//    cpu.rlim_max = 10;
//    cpu.rlim_cur = 10;
//    mem.rlim_max = 50;
//    mem.rlim_cur = 50;
//
//    setrlimit(RLIMIT_CPU, &cpu);
//    setrlimit(RLIMIT_AS,  &mem);
//
//    getrlimit(RLIMIT_CPU, &limit);
//    printf("Proces %d: CPU - hard: %d, CPU - soft: %d\n", getpid(), limit.rlim_max, limit.rlim_cur);
//    getrlimit(RLIMIT_AS, &limit);
//    printf("Proces %d: MEM - hard: %d, MEM - soft: %d\n", getpid(), limit.rlim_max, limit.rlim_cur);



    char list[20];
    strcpy(list, argv[1]);
    int num_of_processes = atoi(argv[2]);
    int max_time = atoi(argv[3]);
    int type = -1;
    if(strcmp(argv[4], "com") == 0){
        type = 1;
    }
    else type = 0;
    int cpu_limit = atoi(argv[5]);
    int mem_limit = atoi(argv[6]);

    FILE* list_file = fopen(list, "r");
    int pairs = count_rows(list_file);
    fclose(list_file);

    // Count columns of second matrix:
    int* columns = calloc(pairs, sizeof(int));
    for(int i=0; i<pairs; i++){
        char* matrix2 = read_matrix_filename(list, i+1, 2);
        FILE* second_matrix = fopen(matrix2, "r");
        columns[i] = count_columns(second_matrix);
    }


    //Tasks for processes - first_column[process][matrix_num] indexing columns from 1 to n;
    int** first_column = calloc(num_of_processes, sizeof(int*)); // first_column of matrix
    int** column_count = calloc(num_of_processes, sizeof(int*));
    int* proc_id = calloc(num_of_processes, sizeof(int));
    int* results = calloc(num_of_processes, sizeof(int));
    for(int i=0; i<num_of_processes; i++){
        first_column[i] = calloc(pairs, sizeof(int));
        column_count[i] = calloc(pairs, sizeof(int));
    }

    for(int pair=0; pair < pairs; pair++){
        int each = (columns[pair] / num_of_processes);
        int additional = (columns[pair] % num_of_processes);
        int tmp_col = 1;
        for(int p = 0; p< num_of_processes; p++){
            if(p < additional){
                first_column[p][pair] = tmp_col;
                column_count[p][pair] = each + 1;
                tmp_col += each + 1;
            }
            else {
                first_column[p][pair] = tmp_col;
                column_count[p][pair] = each;
                tmp_col += each;
            }
        }
    }

    for(int p=0; p< num_of_processes; p++){
        pid_t child_pid;
        child_pid = fork();
        if(child_pid!=0){
            proc_id[p] = child_pid;
        }
        else{
            child_process(p, column_count[p], first_column[p], max_time, type, pairs, list, cpu_limit, mem_limit);
        }

    }
    int res=-1;
    for(int p=0; p< num_of_processes; p++){
        int pid = wait(&res);
        int status = WEXITSTATUS(res);
        if(status == 0){
            printf("Proces o PID: %d wyczerpał zasoby i zostal przerwany.\n", pid, status);
        }
        else{
        printf("Proces o PID: %d wykonał %d mnożeń macierzy.\n", pid, status);
        }
    }

    // FILES CONCATENATION
    if(type == 0){
        char** output = calloc(pairs, sizeof(char*));
        for(int p=1; p<=pairs; p++){
            output[p-1] = read_matrix_filename(list, p , 3);
        }
        char* tmp_col_filename = calloc(20, sizeof(char));
        char** params;
        for(int pair = 1; pair <= pairs; pair++) {
            int params_num = columns[pair - 1] + 1;
            params = calloc(params_num + 2, sizeof(char *));
            for (int i = 0; i < params_num + 1; i++) {
                params[i] = calloc(25, sizeof(char));
            }
            strcpy(params[1], "--delimiter=  ");
            for (int col = 1; col <= params_num - 1; col++) {
                sprintf(tmp_col_filename, "tmp_m%03d_part%03d", pair, col);
                FILE *tmp_col = fopen(tmp_col_filename, "r");
                if (tmp_col == NULL) {
                    printf("Kolumna %d macierzy %d nie została obliczona!\n", col, pair);
                    strcpy(params[col + 1], "");
                } else {
                    strcpy(params[col + 1], tmp_col_filename);
                }
                if (tmp_col != NULL) { fclose(tmp_col); }
            }
            params[params_num + 1] = NULL;
            int out = fileno(stdout);
            int fd = open(output[pair - 1], O_RDWR | O_CREAT | O_TRUNC, 0644);
            dup2(fd, out);

            if (fork() == 0) {
                execvp("paste", params);
                exit(0);
            } else {
                close(fd);
                wait(NULL);
            }
            for (int i = 0; i < params_num + 2; i++) {
                free(params[i]);
            }
            free(params);
        }
        system("rm tmp_*");
    }

    return 0;
}