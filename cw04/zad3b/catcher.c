#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int signals_received = 0;

void sig_setup(void (*f) (int, siginfo_t*, void*)) {
    sigset_t signals;
    sigfillset(&signals);
    sigdelset(&signals, SIGUSR1);
    sigdelset(&signals, SIGUSR2);
    sigprocmask(SIG_BLOCK, &signals, NULL); //sigusr only not belong to mask

    struct sigaction action;
    action.sa_sigaction = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;

    if(sigaction(SIGUSR1, &action, NULL) == -1 || sigaction(SIGUSR2, &action, NULL) == -1) {
        printf("Sth went wrong.\n");
        exit(-1);
    }
}

void sig_setup_rt(void (*f) (int, siginfo_t*, void*)) {
    sigset_t signals;
    sigfillset(&signals);
    sigdelset(&signals, SIGRTMIN);
    sigdelset(&signals, SIGRTMAX);

    sigprocmask(SIG_BLOCK, &signals, NULL); // sigrt only not belong to mask

    struct sigaction action;
    action.sa_sigaction = f;
    action.sa_flags = SA_SIGINFO;

    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGRTMIN);
    sigaddset(&action.sa_mask, SIGRTMAX);

    if(sigaction(SIGRTMIN, &action, NULL) == -1 || sigaction(SIGRTMAX, &action, NULL) == -1) {
        printf("Sth went wrong.\n");
        exit(-1);
    }
}

void kill_catcher(int sig_num, siginfo_t *info, void *context) {
    if(sig_num == SIGUSR1) {
        signals_received++;
        if(kill(info->si_pid, SIGUSR1) != 0){
            printf("Sth went wrong.\n");
            exit(-1);
        }
    }
    else if(sig_num == SIGUSR2) {
        if(kill(info->si_pid, SIGUSR2) != 0){
            printf("Sth went wrong.\n");
            exit(-1);
        }
        exit(0);
    }
}

void queue_catcher(int sig_num, siginfo_t *info, void *context) {
    if(sig_num == SIGUSR1) {
        signals_received++;
        if(sigqueue(info->si_pid, SIGUSR1, info->si_value) != 0){
            printf("Sth went wrong.\n");
            exit(-1);
        }
    }
    else if(sig_num == SIGUSR2) {
        if (kill(info->si_pid, SIGUSR2) != 0) {
            printf("Sth went wrong.\n");
            exit(-1);
        }
        exit(0);
    }
}


void rt_catcher(int sig_num, siginfo_t *info, void *context) {
    if(sig_num == SIGRTMIN) {
        signals_received++;
        if(kill(info->si_pid, SIGRTMIN) != 0){
            printf("Sth went wrong.\n");
            exit(-1);
        }
    }
    else if(sig_num == SIGRTMAX){
        if(kill(info->si_pid, SIGRTMAX) != 0) {
            printf("Sth went wrong.\n");
            exit(-1);
        }
        exit(0);
    }
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Wrong arguments. Input: mode.\n");
        return -1;
    }
    char *mode = argv[1];

    if(strcmp(mode, "KILL") == 0) {
        sig_setup(kill_catcher);
    }
    else if(strcmp(mode, "SIGQUEUE") == 0) {
        sig_setup(queue_catcher);
    }
    else if(strcmp(mode, "SIGRT") == 0) {
        sig_setup_rt(rt_catcher);
    }
    else {
        printf("Wrong mode! There are 3 possible modes: KILL/SIGQUEUE/SIGRT.\n");
        return -1;
    }

    printf("CATCHER_PID: %d\n", getpid());

    while(1) {
        pause();
    }

    return 0;
}