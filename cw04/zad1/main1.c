#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int status;

void sigint_handler(int sig_num) {
    printf("\nOdebrano sygnal SIGINT.\n");
    exit(0);
}

void sigtstp_handler(int sig_num){
    if (status == 0){
        printf("\nOczekuję na CTRL+Z - kontynuacja albo CTRL+C - zakończenie programu.\n");
        status = 1;
    }
    else status = 0;
}

int main(int argc, char **argv) {

    time_t current_time;
    signal(SIGINT, sigint_handler);

    struct sigaction action;
    action.sa_handler = sigtstp_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGTSTP, &action, NULL);

    while(1) {
        if(status == 0) {
            time(&current_time);
            printf("%s", asctime(localtime(&current_time)));
        }
    }
}