#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int finish = 0;
int received_signals=0;
int idx = -1;

void handler_usr1(){
    received_signals++;
}

void handler_usr2(){
    finish = 1;
}
void handler_usr2_sigquque(int sig, siginfo_t* info, void* ucontext){
    finish = 1;
    idx = info->si_value.sival_int;
}
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

int main(int argc, char *argv[]) {
    if(argc < 4){
        printf("Not enought arguments.\n");
        printf("-----------------------------------------\n");
        printf("Usage: ./sender <pid> <count> <mode>\n");
        printf("pid - pid of catcher program \n");
        printf("count - number of signals to send \n");
        printf("mode - [kill|sigqueue|sigrt]\n");
        return 0;
    }
    printf("Sender PID: %d\n", getpid());
    pid_t pid = -1;
    int count = -1;
    int mode =0;
    sscanf(argv[1], "%d", &pid);
    sscanf(argv[2], "%d", &count);
    if(strcmp("kill", argv[3]) == 0) mode = 0;
    else if(strcmp("sigqueue", argv[3]) == 0) mode = 1;
    else if(strcmp("sigrt", argv[3]) == 0) mode = 2;



    if(mode == 1){
        signal(SIGUSR1, handler_usr1);
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler_usr2_sigquque;
        sigaction(SIGUSR2, &sa, NULL);

    }
    else if(mode == 0){
        signal(SIGUSR1, handler_usr1);
        signal(SIGUSR2, handler_usr2);
    }
    else{
        signal(SIGRTMIN, handler_usr1);
        signal(SIGRTMIN+1, handler_usr2);
    }
    for(int i = 0; i < count; i++) {
        send_signal_usr1(pid, mode, i);
        pause();
    }
    send_signal_usr2(pid, mode, count);

    printf("Wysłano: %d sygnałów. \n", count);


    return 0;
}
