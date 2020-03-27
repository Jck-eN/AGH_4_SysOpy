#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int **data;
    int rows;
    int cols;
} matrix;

#define MAX_NUMBER 100

void write_matrix(char* filename, matrix* m){
    FILE* output = fopen(filename, "w+");
    if(!output){
        printf("Error opening file! (write_matrix())");
        exit(1);
    }
    srand(time(NULL));
    for(int i=0; i<m->rows; i++){
        for(int j=0; j < m->cols; j++) {
            fprintf(output, "%09d", m->data[i][j]);
            if(j!=m->cols-1) fprintf(output, " ");
        }
        putc('\n', output);
    }
    fclose(output);
}

void write_random_matrix(char* filename, int n_rows, int n_cols){
    FILE* output = fopen(filename, "w+");
    if(!output){
        printf("Error opening file! (write_random_matrix())");
        exit(1);
    }
    for(int j=0; j<n_rows; j++){
        for(int i=0; i < n_cols; i++) {
            fprintf(output, "%09d", rand() % MAX_NUMBER);
            if(i!=n_cols-1) fprintf(output, " ");
        }
        putc('\n', output);
    }
    fclose(output);
}

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
    for (char c = getc(file); c != '\n'; c = getc(file))
        if (c == ' ')
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

void create_list(int pairs){
    FILE* output = fopen("lista", "w+");
    if(!output){
        printf("Error opening file! (create_list())");
        exit(1);
    }
    for(int j=1; j<=pairs; j++){
            fprintf(output, "m_%d_1 m_%d_2 res_%d\n", j, j, j);
    }
    fclose(output);
}

void create_empty_file(char* name, int rows, int cols){
    FILE* new = fopen(name, "w");
    for(int i=0; i<rows; i++){
        for(int j=0; j< cols*10-1; j++) {
            fputc(' ', new);
        }
        fputc('\n', new);
    }
    fclose(new);
}

int main(int argc, char* argv[]) {
    if(argc < 4){
        printf("Not enought arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./generate_matrices <num_of_pairs> <min> <max>\n");
        printf("num_of_pairs - number of pairs of files with matrices \n");
        printf("min - minimal size of matrix \n");
        printf("max - maximal size of matrix \n");
        return 0;
    }
    srand(time(NULL));
    int pairs = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);
    int cols1, cols2, rows1, rows2;
    char filename[10];
    srand(time(NULL));
    for(int i=1; i<=pairs; i++){
        rows1 = rand() % (max-min+1) + min;
        cols1 = rows2 = rand() % (max-min+1) + min;
        cols2 = rand() % (max-min+1) + min;
        sprintf(filename, "m_%d_%d", i, 1);
        write_random_matrix(filename, rows1, cols1);
        sprintf(filename, "m_%d_%d", i, 2);
        write_random_matrix(filename, rows2, cols2);
        sprintf(filename, "res_%d", i);
        create_empty_file(filename, rows1, cols2);

    }
    create_list(pairs);
    return 0;
}
