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

#define MAX_NUMBER 10000

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
    for (char c = getc(file); c != '\n' && c!= EOF; c = getc(file))
        if (c == ' ')
            cols++;
    return cols+1;
}

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
    return result;
}

matrix* multiply_matrix(matrix* source1, matrix* source2){
    matrix* result = calloc(1, sizeof(matrix));
    result->cols = source2->cols;
    result->rows = source1->rows;
    result->data = calloc(result->rows, sizeof(int*));
    for(int i=0; i< result->rows; i++){
        result->data[i] = calloc(result->cols, sizeof(int));
        for(int j=0; j< result->cols; j++){
            int sum = 0;
            for(int k = 0; k<source1->cols; k++){
                sum += source1->data[i][k]*source2->data[k][j];
            }
            result->data[i][j] = sum;
        }
    }
    return result;
}

int compare_matrices(matrix* source1, matrix* source2) {
    if(source1 == NULL || source2 == NULL || source1->cols != source2->cols || source1->rows != source2->rows) return -1;
    for(int row = 0; row < source1->rows; row++){
        for(int col = 0; col < source1->cols; col++){
            if(source1->data[row][col] != source2->data[row][col]){
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if(argc < 2){
        printf("Not enought arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./tester <list_file>\n");
        printf("list_file - name of file with matrix list \n");
        return 0;
    }
    FILE* list = fopen(argv[1], "r");
    int pairs = count_rows(list);
    fclose(list);
    matrix** source1 = calloc(pairs, sizeof(matrix*));
    matrix** source2 = calloc(pairs, sizeof(matrix*));
    matrix** dest = calloc(pairs, sizeof(matrix*));
    matrix** res = calloc(pairs, sizeof(matrix*));
    for(int i=0; i< pairs; i++){
        source1[i] = read_matrix(read_matrix_filename(argv[1], i+1, 1));
        source2[i] = read_matrix(read_matrix_filename(argv[1], i+1, 2));
        dest[i] = read_matrix(read_matrix_filename(argv[1], i+1, 3));
    }
    for(int i=0; i< pairs; i++){
        res[i] = multiply_matrix(source1[i], source2[i]);
    }
    int correct = 1;
    printf("\n");
    for(int i=0; i< pairs; i++){
        if(compare_matrices(res[i], dest[i]) != 1){
            correct = 0;
            printf("Result matrix number %d is incorrect!\n", i+1);
        }
        else{
            printf("Result matrix number %d is correct!\n", i+1);
        }
    }
    if(correct == 1){
        printf("All matrices are correct!");
    }
    return 0;
}