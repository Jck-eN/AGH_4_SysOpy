#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int run =1;


void handlerSIGTSTP(int num){
    if(run == 1) run =0;
    else run = 1;
    puts("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu");
}

void handlerSIGINT(int num){
    printf("\nOdebrano sygnał SIGINT. Do widzenia!\n", num);
    exit(0);
}

int main() {
    signal(SIGINT, handlerSIGINT);

    struct sigaction sa;
    sa.sa_handler = handlerSIGTSTP;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("Couldn't set SIGTSTP handler");
        exit(EXIT_FAILURE);
    }

    while (1){
        while (run) {
            system("clear");
            system("ls -l");
            sleep(1);
        }
        sleep(1);
    }
    return 0;
}
