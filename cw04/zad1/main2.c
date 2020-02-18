#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

pid_t pid;

void sigint_handler(int sig_num) {

    printf("\nOdebrano sygnal SIGINT.\n");
    if(waitpid(pid, NULL, WNOHANG) == 0)  kill(pid, SIGKILL);
    exit(0);

}

void sigtstp_handler(int sig_num) {

    if(waitpid(pid, NULL, WNOHANG) == 0) {
        printf("\nOczekuję na CTRL+Z - kontynuacja, albo CTRL+C - zakończenie programu.\n");
        kill(pid, SIGKILL); // kill child
    }
    else {
        pid = fork(); // make new child
        if (pid == 0) execl("script.sh", "script.sh", NULL); // if child exec script.sh
    }
}


int main(int argc, char **argv) {

    signal(SIGINT, sigint_handler);

    struct sigaction action;
    action.sa_handler = sigtstp_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGTSTP, &action, NULL);

    pid = fork();

    //if child exec script.sh
    if(pid == 0) {
        execl("script.sh", "script.sh", NULL);
        exit(0);
    }
    while(1);

}