#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    if(argc < 4){
        printf("Not enough arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./producer <pipe> <data> <number_of_chars>\n");
        printf("pipe - path to FIFO queue\n");
        printf("data - path to file with data\n");
        printf("number_of_chars - number of chars read from data file in one step\n");
    }

    char pipe_path[100];
    char data_path[100];

    strcpy(pipe_path, argv[1]);
    strcpy(data_path, argv[2]);
    int num_chars = atoi(argv[3]);


    int len = 0;
    char buff[num_chars+1];
    char output[num_chars+15];
    int pid = getpid();

    FILE* data = fopen(data_path, "r");
    if(data == NULL){
        printf("Couldn't open file with data!");
        exit(1);
    }
    srand(time(NULL));

    int fifo = open(pipe_path, O_WRONLY);
    if(fifo < 0){
        printf("Couldn't open fifo file!");
        exit(1);
    }

    len = fread(&buff, sizeof(char), num_chars, data);
    while(len > 0){
        buff[len] = '\0';
        sprintf(output,"#%d#%s", pid, buff);
        write(fifo, output, strlen(output));
        int time = rand()%1000000;
        usleep(1000000+time); // Sleeps for 1-2 seconds
        len = fread(&buff, sizeof(char), num_chars, data);
    }

    fclose(data);
    close(fifo);

    return 0;
}
