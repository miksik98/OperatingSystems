#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>



#define LINE_MAX 512
#define CMDS_MAX 32

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Wrong number of args! The only arg should be a file_path.\n");
        return -1;
    }

    FILE *file;
    if((file = fopen(argv[1], "r")) == NULL){
        printf("Could not open a file!\n");
        return -1;
    }

    int fd[2][2];
    char line[LINE_MAX];
    char* commands[CMDS_MAX][CMDS_MAX];
    pid_t pids[CMDS_MAX];
    while(fgets(line, sizeof(line), file)) {
        char* cmd = strtok(line, " \n");
        //saving every command
        int i, j;
        for(j = 0, i = 0; cmd != NULL; j++, cmd = strtok(NULL, " \n")) {
            if(strcmp(cmd, "|") == 0) {
                commands[i][j] = NULL;
                i++;
                j = -1;
            } else commands[i][j] = cmd;
        }

        int n = i+1;
        for(i = 0; i < n; i++) {
            if(pipe(fd[1]) < 0) {
                printf("Sth went wrong with pipe.\n");
                return -1;
            }

            if((pids[i]=fork()) == 0) {
                if(i != 0) {
                    dup2(fd[0][0], STDIN_FILENO);
                    close(fd[0][0]);
                    close(fd[0][1]);
                }
                if(i != n-1) {
                    dup2(fd[1][1], STDOUT_FILENO);
                    close(fd[1][0]);
                    close(fd[1][1]);
                }
                execvp(commands[i][0], commands[i]);
                exit(0);
            } else {
                if(i != 0) {
                    close(fd[0][0]);
                    close(fd[0][1]);
                }
                if(i != n-1) {
                    fd[0][0] = fd[1][0];
                    fd[0][1] = fd[1][1];
                }
            }
        }

        for(i = 0; i < n-1; i++) { //pids[n-1] closes file & return 0
            int status;
            waitpid(pids[i], &status, 0);
        }
    }

    fclose(file);
    return 0;
}
