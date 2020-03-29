#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

void handler(){
    printf("Received signal!\n");
}


int main(int argc, char *argv[]) {
    if(argc < 3){
        printf("Not enought arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./main <mode> <mode2>\n");
        printf("mode - [ignore|handler|mask|pending]\n");
        printf("mode2 - [child|exec]\n");
        return 0;
    }
    int mode =0, mode2 = 0;
    if(strcmp("ignore", argv[1]) == 0) mode = 0;
    else if(strcmp("handler", argv[1]) == 0) mode = 1;
    else if(strcmp("mask", argv[1]) == 0) mode = 2;
    else if(strcmp("pending", argv[1]) == 0) mode = 3;
    if(strcmp("child", argv[2]) == 0) mode2 = 0;
    else if(strcmp("exec", argv[2]) == 0) mode2 = 1;


    sigset_t sigset;

    if(mode == 0){
        signal(SIGUSR1, SIG_IGN);

    }
    else if(mode == 1){
        signal(SIGUSR1, handler);
    }
    else if(mode == 2 || mode == 3){
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGUSR1);
        sigprocmask(SIG_BLOCK, &sigset, NULL);
    }

    raise(SIGUSR1);
    sigset_t pending_set;
    if (mode == 2 || mode == 3)
    {
        sigpending(&pending_set);
        printf("Signal pending (main): %d\n", sigismember(&pending_set, SIGUSR1));
    }

    if(mode2 == 0){
        pid_t child_pid = fork();
        if(child_pid == 0){
            if(mode != 3){
                raise(SIGUSR1);
            }
            if (mode == 2 || mode == 3)
            {
                sigpending(&pending_set);
                printf("Signal pending (child): %d\n", sigismember(&pending_set, SIGUSR1));
            }
        }
    }
    else{
        execl("./second", "./second", argv[1], NULL);
    }


    return 0;
}