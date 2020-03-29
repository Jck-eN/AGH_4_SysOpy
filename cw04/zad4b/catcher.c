#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int finish = 0;
int received_signals=0;
int idx = -1;
pid_t pid_sender = -1;
int mode = 0;



void send_signal_usr1(int pid, int mode, int number) {
    union sigval value;
    switch(mode) {
        case 0:
            kill(pid, SIGUSR1);
            break;
        case 1:
            value.sival_int = number;
            sigqueue(pid, SIGUSR1, value);
            break;
        case 2:
            kill(pid, SIGRTMIN);
            break;
    }
}

void send_signal_usr2(int pid, int mode, int number) {
    union sigval value;
    switch(mode) {
        case 0:
            kill(pid, SIGUSR2);
            break;
        case 1:
            value.sival_int = number;
            sigqueue(pid, SIGUSR2, value);
            break;
        case 2:
            kill(pid, SIGRTMIN+1);
            break;
    }
}

void handler_usr1(int num, siginfo_t* info, void* ucontext){
    pid_sender = info->si_pid;
    received_signals++;
    send_signal_usr1(pid_sender, mode, -1);
}

void handler_usr2(int num, siginfo_t* info, void* ucontext){
    finish = 1;
    pid_sender = info->si_pid;
}


int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("Not enought arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./catcher <mode>\n");
        printf("mode - [kill|sigqueue|sigrt]\n");
        return 0;
    }
    printf("Catcher PID: %d\n", getpid());
    mode =0;
    if(strcmp("kill", argv[1]) == 0) mode = 0;
    else if(strcmp("sigqueue", argv[1]) == 0) mode = 1;
    else if(strcmp("sigrt", argv[1]) == 0) mode = 2;



    if(mode == 2){
        struct sigaction sa1;
        sa1.sa_flags = SA_SIGINFO;
        sa1.sa_sigaction = handler_usr1;
        sigaction(SIGRTMIN, &sa1, NULL);
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler_usr2;
        sigaction(SIGRTMIN+1, &sa, NULL);

    }
    else{
        struct sigaction sa1;
        sa1.sa_flags = SA_SIGINFO;
        sa1.sa_sigaction = handler_usr1;
        sigaction(SIGUSR1, &sa1, NULL);

        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler_usr2;
        sigaction(SIGUSR2, &sa, NULL);
    }
    while(!finish) pause();

    printf("Otrzymano %d sygnały\n", received_signals);

    return 0;
}
