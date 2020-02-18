#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

typedef struct{
    char *filename;
    pid_t pid;
    int running;
} child_process;

int stop = 0;

int is_a_number(char * string){
    for (int i = 0; i < strlen(string); i++){
        if (((int)string[i]>57)||((int)string[i]<48)) return 1; // not digit
    }
    return 0;
}

int get_mod_date(char *file, time_t *times){
    struct stat s;

    if(lstat(file,&s) == 0){
        *times = s.st_mtime; //modification date from lstat
        return 0;
    }
    return -1;
}

int get_copy_name(char *file_name, time_t mod_date, char **copy_name){
    char *parsed_date = malloc(21 * sizeof(char));
    strftime(parsed_date,21,"_%Y-%m-%d_%H-%M-%S",localtime(&mod_date));
    *copy_name = malloc(strlen(basename(file_name))+30); //strlen(parsed_date + "archiwum/") = 30
    sprintf(*copy_name,"archiwum/%s%s",basename(file_name),parsed_date);
    return 0;
}

int get_file_text(char *file_name, char **text){
    FILE *file = fopen(file_name,"r");
    if (file == NULL) return -1;
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *text = malloc(size * sizeof(char));
    fread(*text,sizeof(char),size,file);
    return fclose(file);
}

int save_copy(char *file_name, char *text){
    FILE *file = fopen(file_name,"w");
    if (file == NULL) return -1;
    fwrite(text,sizeof(char),strlen(text),file);
    return fclose(file);
}

int monitor(char *file_name, int time, int freq){
    int copies = 0;
    time_t mod_date;
    char *copy_name;
    char *text;

    if(get_mod_date(file_name,&mod_date) != 0) return 0;
    if(get_file_text(file_name,&text) != 0) return 0;

    time_t last_mod_date;
    int delta_time = 0;

    while(time>0){
        sleep(1);
        delta_time++;
        if(delta_time == freq){
            delta_time = 0;

            if(get_mod_date(file_name,&last_mod_date)!=0)  return copies;

            if(last_mod_date != mod_date){ //=> zmodyfikowano plik
                if (get_copy_name(file_name,mod_date,&copy_name)!=0) return copies;
                if (save_copy(copy_name,text) != 0) return copies;

                mod_date = last_mod_date;
                copies++;
            }
        }
        time --;
    }
    return copies;
}

int end = 0;
void sigint_handler(int sig_num){
    end = 1;
}

void sigusr1_handler(int sig_num){
    if(stop == 0) stop = 1;
    else stop = 0;
}

int list(child_process *children, int length){

    if(children == NULL) return -1;

    for(int i = 0; i < length; i++){
        if(children[i].running == 1){
            printf("Process with PID = %d is currently monitoring file \'%s\'\n", children[i].pid, children[i].filename);
        }
        else{
            printf("Stopped process with PID = %d is monitoring file \'%s\'\n", children[i].pid, children[i].filename);
        }
    }
    return 0;
}

int stop_process(child_process *child){
    if(child == NULL){
        printf("Child is null\n");
        return -1;
    }

    if(child->running == 0){
        printf("%d already stopped\n", child->pid);
        return 1;
    }

    if(kill(child->pid, SIGUSR1) == -1){
        printf("%d sigusr1 returned -1\n", child->pid);
        return -1;
    }
    else{
        child->running = 0;
        printf("STOPPED PROCESS %d\n", child->pid);
    }
    return 0;
}

int stop_pid(child_process *children, int length, pid_t pid){
    if (children == NULL) return -1;

    child_process *target = NULL;
    for (int i = 0; i < length; i++){
        if (children[i].pid == pid){
            target = &children[i];
        }
    }

    if (target != NULL) return stop_process(target);
    return -1;
}

int stop_all(child_process *children, int length){
    if (children == NULL) return -1;

    for (int i = 0; i < length; i++){
        stop_process(&children[i]);
    }

    return 0;
}

int start_process(child_process *child){
    if (child == NULL){
        printf("NULL\n");
        return -1;
    }

    if (child->running == 1){
        printf("%d already running\n", child->pid);
        return 1;
    }

    if (kill(child->pid, SIGUSR1) == -1) return -1;
    else child->running = 1;
    printf("STARTED PROCESS %d\n",child->pid);
    return 0;
}

int start_pid(child_process *children, int length, pid_t pid){
    if (children == NULL) return -1;

    child_process *target = NULL;
    for (int i = 0; i < length; i++){
        if (children[i].pid == pid){
            target = &children[i];
        }
    }

    if (target != NULL) return start_process(target);

    return -1;
}

int start_all(child_process *children, int length){
    if (children == NULL) return -1;

    for (int i = 0; i < length; i++) start_process(&children[i]);

    return 0;
}

int main(int argc, char ** argv) {

    if (argc != 3) {
        printf("Wrong number of args!\nInput: list time.\n");
        return 1;
    }
    char *listfile = argv[1];
    FILE *file = fopen(listfile, "r");
    if(file == NULL){
        printf("Something went wrong with opening file: %s.\n",listfile);
        return 1;
    }

    if(is_a_number(argv[2])!=0){
        printf("2nd arg should be nonnegative numbers.\n");
        return 1;
    }

    int time = atoi(argv[2]);

    fseek(file,0,SEEK_END);
    int list_size = ftell(file);
    fseek(file,0,SEEK_SET);

    struct sigaction action;
    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);

    char *paths = malloc(list_size*sizeof(char));
    int path_counter = 0;
    fread(paths, sizeof(char),list_size,file);

    char *s = paths;
    int lines;
    for (lines=0; s[lines]; s[lines]=='\n' ? lines++ : *s++);
    child_process *children = calloc(lines, sizeof(child_process));




    char *current_path = strtok(paths, " \n");

    while (current_path!=NULL){
        int freq = atoi(strtok(NULL, " \n"));
        pid_t pid = fork();
        if (pid==0) {
            struct sigaction child_action;
            child_action.sa_handler = sigusr1_handler;
            sigemptyset(&child_action.sa_mask);
            sigaction(SIGUSR1, &child_action, NULL);
            return monitor (current_path,time,freq);
        }
        else{
            child_process *forked = &children[path_counter];
            forked->filename = current_path;
            forked->pid = pid;
        }
        current_path = strtok(NULL," \n");
        path_counter++;
    }

    char cmd[50];

    while(!end){
        fgets(cmd, 50, stdin);

        if (strcmp(cmd, "LIST\n") == 0) {
            if (list(children, path_counter) == -1) printf("Sth went wrong.\n");
        }
        else if (strcmp(cmd, "STOP ALL\n") == 0){
            if(stop_all(children, path_counter) != 0) printf("Sth went wrong.\n");
        }
        else if (strncmp(cmd, "STOP ", 5) == 0){
            int pid = atoi(cmd+5);
            if (pid != 0){
                if (stop_pid(children, path_counter, pid) != 0) printf("Sth went wrong.\n");
            }
        }
        else if (strcmp(cmd, "START ALL\n") == 0){
            if (start_all(children, path_counter) != 0) printf("Sth went wrong.\n");
        }
        else if (strncmp(cmd, "START ", 6) == 0) {
            int pid = atoi(cmd + 6);
            if (pid != 0) {
                if (start_pid(children, path_counter, pid) != 0) printf("Sth went wrong.\n");
            } else {
                printf("There is no process with PID = %s\n", cmd + 6);
                return 1;
            }
        }
        else {
            if (strcmp(cmd, "END\n") == 0) {
                end = 1;
            } else {
                printf("Wrong command!\n");
                return 1;
            }
        }
    }

    while(path_counter>0){
        int status;
        pid_t pid = wait(&status);
        if(pid >0) printf("\nPID %d created %d copies.\n\n",pid, WEXITSTATUS(status));
        else{
            printf("Something went wrong with changing process status.\n");
            return 1;
        }
        path_counter--;
    }

    return 0;
}
