#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

void handler_child(int num, siginfo_t* info, void* ucontext){
    printf("Received signal number %d - SIGCHLD. Child process with id %d exited with status %d.\n",
            info->si_signo,
            info->si_pid,
            info->si_status);
}

void handler_queue(int num, siginfo_t* info, void* ucontext){
    printf("Received signal number %d - SIGRTMIN. Sent by process with id %d. Value received: %d\n",
           info->si_signo,
           info->si_pid,
           info->si_value);
}

void handler_int(int num, siginfo_t* info, void* ucontext){
    printf("Received signal number %d - SIGINT. Sent by process with id %d. Send by ",
           info->si_signo,
           info->si_pid);
    if(info->si_code == SI_USER){
        printf("user.\n");
    } else if (info->si_code == SI_KERNEL){
        printf("kernel.\n");
    } else if (info->si_code == SI_QUEUE){
        printf("sigqueue.\n");
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("Not enought arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./main <mode>\n");
        printf("mode - [queue|child|int]\n");
        return 0;
    }
    int mode =0;
    if(strcmp("queue", argv[1]) == 0) mode = 0;
    else if(strcmp("child", argv[1]) == 0) mode = 1;
    else if(strcmp("int", argv[1]) == 0) mode = 2;

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);


    if(mode == 0){
        union sigval sv;
        sv.sival_int = 123456;
        sa.sa_sigaction = handler_queue;
        sigaction(SIGRTMIN, &sa, NULL);
        sigqueue(getpid(), SIGRTMIN, sv);

    }
    else if(mode == 1){
        sa.sa_sigaction = handler_child;
        sigaction(SIGCHLD, &sa, NULL);
        pid_t child_pid = fork();
        if(child_pid == 0){
            exit(111);
        }
        wait(NULL);
    }

    else {
        union sigval sv;
        sa.sa_sigaction = handler_int;
        sigaction(SIGINT, &sa, NULL);
        sleep(2);
        sigqueue(getpid(), SIGINT, sv);
    }

    return 0;
}
