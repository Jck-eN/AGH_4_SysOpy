#include "lib_diff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct main_arr* create_main_arr(int blocks_size){
    if (blocks_size <= 0) return NULL;

    struct main_arr* array = (struct main_arr*) calloc(1, sizeof(struct main_arr));
    if (array == NULL){
        printf("Error. Cannot allocate main table.");
        return NULL;

    }
    array->blocks_size = blocks_size;
    array->last_block = -1;

    array->blocks = (struct operations_block**) calloc(blocks_size, sizeof(struct operations_block*));

    return array;

}

struct files_pair* create_files_pair(char* paths){
    char* divide = strchr(paths, ':');
    if (divide == NULL) {
        printf("Error! Wrong files path format. Must be <path1>:<path2> ");
        return NULL;
    }


    int first_path_length = divide - paths;


    struct files_pair* pair = (struct files_pair*) calloc(1, sizeof(struct files_pair));
    
    pair->first_path = (char*) calloc(first_path_length+1, sizeof(char));
    pair->second_path = (char*) calloc(strlen(divide+1)+1, sizeof(char));

    strncpy(pair->first_path, paths, first_path_length);
    strcpy(pair->second_path, divide+1);

    return pair;
}

struct pairs_sequence* create_files_sequence(int pairs_count, char** pairs){
    struct pairs_sequence* sequence = (struct pairs_sequence*) calloc(1, sizeof(struct pairs_sequence));
    sequence->pairs =(struct files_pair**) calloc(pairs_count, sizeof(struct files_pair*));
    sequence->pairs_number = pairs_count;

    for(int i=0; i< pairs_count; i++){
        sequence->pairs[i] = create_files_pair(pairs[i]);
    }

    return sequence;
}


void compare_sequence(struct main_arr* arr, struct pairs_sequence* sequence){

    for(int i=0; i< sequence->pairs_number; i++){
        char* name = "tmp";
        compare_files_and_save_to_file(sequence->pairs[i],name);

        add_block(arr, name);
    }

    return;
};

void compare_sequence_no_add_block(struct main_arr* arr, struct pairs_sequence* sequence){

    for(int i=0; i< sequence->pairs_number; i++){
        char* name = "tmp";
        compare_files_and_save_to_file(sequence->pairs[i],name);
    }
    return;
};


void compare_files_and_save_to_file(struct files_pair* pair, char* filename) {
    int size = strlen(pair->first_path) + strlen(pair->second_path) + strlen(filename) + 20;
    char command[size];
    
    snprintf(command, size, "diff %s %s > %s", pair->first_path, pair->second_path, filename);

    int diff_result = system(command);

    if (diff_result == -1) {
        printf("Error. Problem occurred during diff execution.\n");
        return;
    }

    return;
}

int add_block(struct main_arr* arr, char* tmp_filename) {
    struct operations_block* block = create_block(tmp_filename);
    if (block == NULL || arr == NULL) return -1;


    arr->blocks[arr->last_block+1] = block;
    arr->last_block++;
    return arr->last_block;
}


struct operations_block* create_block(char* filename){
     FILE *handle = fopen(filename, "r");
    if (!handle) {
        printf("error occurred while fopen tmp file: %s", filename);
        return NULL;
    }

    struct operations_block* new_block = (struct operations_block *) calloc(1, sizeof(struct operations_block));
    new_block->operations_count = 0;
    new_block->operations = NULL;

    char* operation = NULL;
    char* line_buffer = NULL;

    size_t line_buffer_size = 0;
    size_t operation_allocated_length = 0;
    size_t operation_length = 0; 

    while (getline(&line_buffer, &line_buffer_size, handle) >= 0) {
        if (isdigit(line_buffer[0])) {
            if (operation_allocated_length == 0) {
                operation_allocated_length = strlen(line_buffer) + 1;

                operation = (char*) realloc(operation, operation_allocated_length * sizeof(char));
            } else {
                if (new_block->operations == NULL) {
                    new_block->operations = (char**) calloc(1, sizeof(char *));
                } else {
                    new_block->operations = (char**) realloc(new_block->operations,
                                                               (new_block->operations_count + 1) * sizeof(char *));
                }
                new_block->operations[new_block->operations_count] = (char*) malloc(operation_length * sizeof(char));
                strcpy(new_block->operations[new_block->operations_count], operation);
                for (int i = 0; i < operation_length; i++) {
                    operation[i] = '\0';
                }
                new_block->operations_count++;
            }
            size_t line_buffer_len = strlen(line_buffer);
            if (line_buffer_len > operation_allocated_length) {
                operation = realloc(operation, (line_buffer_len + 1) * sizeof(char));
                operation_allocated_length = line_buffer_len + 1;
            }
            operation_length = line_buffer_len + 1;
            strcpy(operation, line_buffer);
        } else {
            size_t line_buffer_len = strlen(line_buffer);
            if (operation_length + line_buffer_len > operation_allocated_length) {
                operation = (char *) realloc(operation, (operation_allocated_length + line_buffer_len) * sizeof(char));
                operation_allocated_length += line_buffer_len;
            }

            operation_length += line_buffer_len;
            strcat(operation, line_buffer);
        }
    }

    if (operation_length != 0) {
        if (new_block->operations == NULL) {
            new_block->operations = (char **) calloc(1, sizeof(char *));
        } else {
            new_block->operations = (char **) realloc(new_block->operations, (new_block->operations_count + 1) * sizeof(char *));
        }
        new_block->operations[new_block->operations_count] = (char *) malloc(operation_length * sizeof(char));
        strcpy(new_block->operations[new_block->operations_count], operation);
        new_block->operations_count++;
    }

    free(line_buffer);
    free(operation);

    return new_block;
}


int block_size(struct main_arr* arr, int block_idx){

    if(arr->blocks[block_idx] == NULL) return -1;
    return arr->blocks[block_idx]->operations_count;

}

void remove_block(struct main_arr* arr, int block_idx_to_delete){
    if (arr->blocks[block_idx_to_delete] == NULL) {
        return;
    }

    for (int i = 0; i < block_size(arr, block_idx_to_delete); i++) {
        remove_operation(arr->blocks[block_idx_to_delete], i);
    }
    free(arr->blocks[block_idx_to_delete]);
    arr->blocks[block_idx_to_delete] = NULL;

    return ;
}

void remove_operation(struct operations_block* block, int op_idx_to_delete){
    if(!block || block->operations_count <= op_idx_to_delete) return;
    free(block->operations[op_idx_to_delete]);
    block->operations[op_idx_to_delete] = NULL;
    return ;
}