#include <stdio.h>
#include <string.h>
#include "lib_diff.h"


char* proper_commands[3] = {"compare_pairs", "remove_block", "remove_operation"};

int exist_command(char* command){
    for(int i = 0;i<3;i++){
        if(strcmp(command, proper_commands[i]) == 0){
            return i+1;
        }
    }
    return 0;
}

//---------------------------------------
int main(int argc, char const *argv[])
{
    if (argc < 3 || strcmp(argv[1], "create_table") != 0){
        printf("Invalid arguments. Usage:\n");
        printf("./main create_table <size> [commands]\n");
        printf("Commands yout can use:\n");
        printf("compare_pairs file1:file2 file3:file4 ... \n");
        printf("remove_block <index>\n");
        printf("remove_operation <block_index> <operation_index> \n");
    }

    printf("Creating main array\n");
    struct main_arr* tablica = create_main_arr(atoi(argv[2]));
    for (int i = 3; i < argc; i++) {
        int command = exist_command(argv[i]);

        if (command == 1){
            int size = 0;

            while (i+size+1 < argc && exist_command(argv[i+size+1]) == 0) {
                size++;
            }

            char **pair_seq = (char **) calloc(size, sizeof(char *));

            for (int j = 0; j < size; j++) {
                pair_seq[j] = argv[i+j+1];
            }
            printf("Comparing files\n");
            struct pairs_sequence *pairs_seq = create_files_sequence(size, pair_seq);
            compare_sequence(tablica, pairs_seq);

            i+= size;
        }
        else if (command == 2) {
            if(i+1 >= argc){
                printf("Not enought arguments");
                exit(1);
            }
            printf("Removing block\n");
            remove_block(tablica, atoi(argv[++i]));
        } else if (command == 3){
            if(i+2 >= argc){
                printf("Not enought arguments");
                exit(1);
            }
            printf("Removing operation\n");
            remove_operation(tablica->blocks[atoi(argv[++i])], atoi(argv[++i]));
        }

    }
}
