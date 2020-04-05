#include <stdio.h>
#include <unistd.h>


int main(int argc, char* argv[]) {
    if(argc < 2){
        printf("Not enough arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./main <path>\n");
        printf("path - path to file \n");
    }
    char command[50];
    sprintf(&command, "cat %s | sort", argv[1]);

    FILE* output = popen(command, "r");
    char buff[100];
    while(fgets(&buff, 100, output) != NULL){
        printf("%s", buff);
    }
    pclose(output);
    return 0;
}
