#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int signals_expected;
int signals_received = 0;

int is_a_number(char * string){
    for (int i = 0; i < strlen(string); i++){
        if (((int)string[i]>57)||((int)string[i]<48)) return 1; // not digit
    }
    return 0;
}

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

void kill_handler(int sig_num, siginfo_t *info, void *context) {
    if(sig_num == SIGUSR1) {
        signals_received++;
    }
    else if(sig_num == SIGUSR2) {
        printf("Got back %d / %d signals.\n", signals_received, signals_expected);
        exit(0);
    }
}

void queue_handler(int sig_num, siginfo_t *info, void *context) {
    if(sig_num == SIGUSR1) {
        signals_received++;
        printf("%d SIGUSR1 caught.\n", info->si_value.sival_int);
    }
    else if(sig_num == SIGUSR2) {
        printf("Got back %d / %d signals.\n", signals_received, signals_expected);
        exit(0);
    }
}

void sigrt_handler(int sig_num, siginfo_t *info, void *context) {
    if(sig_num == SIGRTMIN) {
        signals_received++;
    }
    else if (sig_num == SIGRTMAX) {
        printf("Got back %d / %d signals.\n", signals_received, signals_expected);
        exit(0);
    }
}

int main(int argc, char **argv) {

    if(argc != 4) {
        printf("Wrong arguments. Input: PID number_of_signals mode.\n");
        return -1;
    }

    if(is_a_number(argv[1])!=0 || is_a_number(argv[2])!=0){
        printf("1st & 2nd arg should be nonnegative numbers!\n");
    }

    int pid = atoi(argv[1]);
    signals_expected = atoi(argv[2]);
    char *mode = argv[3];

    if(strcmp(mode, "KILL") == 0) {
        sig_setup(kill_handler);
        for(int i = 0; i < signals_expected; i++)
            if(kill(pid, SIGUSR1) != 0) {
                printf("SIGUSR1 wasn't sent.\n");
                exit(-1);
            }
        if(kill(pid, SIGUSR2) != 0) {
            printf("SIGURS2 wasn't sent.\n");
            exit(-1);
        }
    }
    else if(strcmp(mode, "SIGQUEUE") == 0) {
        sig_setup(queue_handler);
        for(int i = 0; i < signals_expected; i++) {
            union sigval val = {i};
            if(sigqueue(pid, SIGUSR1, val) != 0) {
                printf("Sth went wrong.\n");
                exit(-1);
            }
        }
        if(kill(pid, SIGUSR2) != 0) {
            fprintf(stderr, "SIGUSR2 wasn't sent.\n");
            exit(-1);
        }
    }
    else if(strcmp(mode, "SIGRT") == 0) {
        sig_setup_rt(sigrt_handler);
        for (int i = 0; i < signals_expected; i++){
            if (kill(pid, SIGRTMIN) != 0) {
                printf("SIGRTMIN wasn't sent.\n");
                exit(-1);
            }
        }
        if(kill(pid, SIGRTMAX) != 0) {
            printf("SIGRTMAX wasn't sent.\n");
            exit(-1);
        }
    }
    else {
        printf("Wrong mode! There are 3 possible modes: KILL/SIGQUEUE/SIGRT.\n");
        return -1;
    }

    while(1) {
        pause();
    }

    return 0;
}