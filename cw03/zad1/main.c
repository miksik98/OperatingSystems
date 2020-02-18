#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <memory.h>
#include<sys/wait.h>

char * start_dir;

void print_relative_path(char * absolute){
    int i = 0;
    while (absolute[i] == start_dir[i]){
        i++;
    }
    i++;
    while (i < strlen(absolute)){
        printf("%c",absolute[i]);
        i++;
    }
    printf("\n");
}

int explore_directory(char * path){

    DIR *dir = opendir(path);
    if (dir == NULL){
        printf("Could not open the directory.\n");
        return 1;
    }

    struct dirent *file= readdir(dir);
    while(file != NULL) {
        char new_path[PATH_MAX];
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, file -> d_name);

        if (strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0){

            struct stat file_stat;

            lstat(new_path, &file_stat);

            if(S_ISDIR(file_stat.st_mode)){
                pid_t pid = fork();
                int status;
                if(pid == 0){
                    printf("%d\n",getpid());
                    print_relative_path(new_path);
                    execlp("/bin/ls","ls","-l",new_path,NULL);
                }
                else {
                    wait(&status);
                }
                explore_directory(new_path);
            }
        }
        file = readdir(dir);
    }
    closedir(dir);
    return 0;
}

int main(int argc, char ** argv) {

    if (argc != 2) {
        printf("Write only 1 param: path.\n");
        return 1;
    }
    char *path = argv[1];
    int last = 0;

    while(argv[1][last] != '\0') last++;

    if(argv[1][last-1] == '/') argv[1][last-1]='\0';

    if(argv[1][0] != '/'){
        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd)) == NULL){
            printf("Could not get cwd.\n");
            return 1;
        };
        if ((argv[1][0] == '.' ) & (argv[1][1] == '/')){
            int i = strlen(cwd);
            for (int j = 1; j < strlen(path); j++){
                cwd[i] = path[j];
                i++;
            }
        }
        else {
            if ((argv[1][0] == '.') & (argv[1][1] == '.')) {
                int i = strlen(cwd);
                while (cwd[i]!='/') i--;
                i++;
                for (int j = 3; j < strlen(path); j++) {
                    cwd[i] = path[j];
                    i++;
                }
            }
            else {
                strcat(cwd, "/");
                strcat(cwd, path);
            }
        }
        path = cwd;
    }

    start_dir = path;

    explore_directory(path);

    return 0;
}
