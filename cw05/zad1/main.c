#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LENGTH 4096
#define MAX_COMMANDS 10
#define MAX_ARGS 10

void execute_line(char* buff){
    if(buff[strlen(buff)-1] == '\n'){
        buff[strlen(buff)-1] = '\0';
    }
    int commands_count = 1;
    for(int i=0; i<strlen(buff); i++ ){
        if(buff[i] == '|'){
            commands_count++;
        }
    }
    char* command[MAX_COMMANDS][MAX_ARGS];
    for(int i=0; i<MAX_COMMANDS; i++){
        for(int j=0; j< MAX_ARGS; j++){
            command[i][j] = NULL;
        }
    }

    int command_id = 0;
    int arg_id = 0;
    char* arg_end;
    char* com_end;
    char* command_tmp = strtok_r(buff, "|", &com_end);
    while(command_tmp != NULL){
        arg_id = 0;
        char* arg_tmp = strtok_r(command_tmp, " ", &arg_end);
        while(arg_tmp != NULL){
            command[command_id][arg_id] = arg_tmp;
            arg_tmp = strtok_r(NULL, " ", &arg_end);
            arg_id++;

        }
        command_tmp = strtok_r(NULL, "|", &com_end);
        command_id++;
    }

    int pipes[commands_count-1][2];
    for(int i=0; i<commands_count-1; i++){
        pipe(pipes[i]);
    }

    for(int i=0; i< commands_count; i++){
        pid_t child_pid = fork();
        if(child_pid == 0) {
            if (i != commands_count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            if (i != 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            for (int j = 0; j < commands_count - 1; j++) {
                close(pipes[j][1]);
            }

            execvp(command[i][0], command[i]);
            exit(1);
        }
    }
    for(int i=0; i < commands_count-1; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for(int i=0; i < commands_count; i++){
        wait(0);
    }

    printf("\n\n");
}

int main(int argc, char* argv[]) {
    if(argc < 2){
        printf("Not enough arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./main <path>\n");
        printf("path - path to file with commands \n");
        return 0;
    }

    FILE* input = fopen(argv[1], "r");
    if(input == NULL){
        printf("Cannot open file %s", argv[1]);
        return(1);
    }
    char* buff = calloc(MAX_LENGTH+1, sizeof(char)) ;
    int len = MAX_LENGTH;
    while(getline(&buff, (size_t *) &len, input) != -1){
        execute_line(buff);
    }

    free(buff);
    return 0;
}
