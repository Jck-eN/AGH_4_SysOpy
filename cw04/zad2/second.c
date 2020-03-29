#include <stdio.h>
#include <signal.h>
#include <string.h>


int main(int argc, char *argv[]) {
    int mode = 0;
    if (strcmp("ignore", argv[1]) == 0) mode = 0;
    else if (strcmp("handler", argv[1]) == 0) mode = 1;
    else if (strcmp("mask", argv[1]) == 0) mode = 2;
    else if (strcmp("pending", argv[1]) == 0) mode = 3;
    if(mode != 3){
        raise(SIGUSR1);
    }
    if(mode == 2 || mode == 3){
        sigset_t sig_pending;
        sigpending(&sig_pending);
        printf("Signal pending (second): %d\n", sigismember(&sig_pending, SIGUSR1));
    }
    return 0;
}

