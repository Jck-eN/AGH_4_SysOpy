#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if(argc < 4){
        printf("Not enough arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./consumer <pipe> <output> <number_of_chars>\n");
        printf("pipe - path to FIFO queue\n");
        printf("output - path to output file\n");
        printf("number_of_chars - number of chars read from data file in one step\n");
    }

    char pipe_path[100];
    char output_path[100];

    strcpy(pipe_path, argv[1]);
    strcpy(output_path, argv[2]);
    int num_chars = atoi(argv[3]);


    int len = 0;
    char buff[num_chars+1];

    FILE* output_file = fopen(output_path, "w+");
    if(output_file == NULL){
        printf("Couldn't create output file!");
        exit(1);
    }
    srand(time(NULL));
    FILE* fifo = fopen(pipe_path, "r");
    if(fifo == NULL){
        printf("Couldn't open fifo file!");
        exit(1);
    }

    len = fread(&buff, sizeof(char), num_chars, fifo);
    while (len > 0) {
        buff[len] = '\0';
        printf("%s", buff);
        fprintf(output_file, "%s", buff);

        len = fread(&buff, sizeof(char), num_chars, fifo);
    }
    fclose(output_file);
    fclose(fifo);

    return 0;
}
