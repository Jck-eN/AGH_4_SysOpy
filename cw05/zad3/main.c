#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char* argv[]) {

    char* consumer[] = {"./consumer", "pipe", "output", "10", NULL};
    char* producer[][5] = {{"./producer", "pipe", "data", "150", NULL},
                           {"./producer", "pipe", "data2","150", NULL},
                           {"./producer", "pipe", "data3", "150", NULL},
                           {"./producer", "pipe", "data4", "150", NULL},
                           {"./producer", "pipe", "data5", "150", NULL}};

    if(fork() == 0)execvp(consumer[0], consumer);

    for(int i=0; i<5; i++){
        if(fork() == 0){
            execvp(producer[i][0], producer[i]);
        }
    }
    for(int i=0; i<6; i++){
        wait(NULL);
    }
    printf("\n\nDone!\n");
    return 0;
}
