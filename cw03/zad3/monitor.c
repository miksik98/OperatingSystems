#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <sys/resource.h>

int mode;

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

int monitor(char *file_name, int time, int freq, rlim_t cpu_limit, rlim_t memory_limit){
    int copies = 0;
    time_t mod_date;
    char *copy_name;
    char *text;

    struct rlimit cpu_rlim;
    struct rlimit memory_rlim;

    cpu_rlim.rlim_cur = cpu_limit;
    cpu_rlim.rlim_max = cpu_limit;

    memory_rlim.rlim_cur = memory_limit;
    memory_rlim.rlim_max = memory_limit;

    if(setrlimit(RLIMIT_CPU, &cpu_rlim) != 0) return 2;
    if(setrlimit(RLIMIT_AS, &memory_rlim) != 0) return 3;

    if(get_mod_date(file_name,&mod_date) != 0) return 0;
    if(mode == 0){
        if(get_file_text(file_name,&text) != 0) return 0;
    }
    else{
        pid_t pid = fork();
        if (pid == 0){
            printf("Copying file: %s\n",file_name);
            if (get_copy_name(file_name,mod_date,&copy_name)!=0) return 0;
            execlp("/bin/cp","cp",file_name, copy_name,NULL);
        }
        copies++;
    }

    time_t last_mod_date;
    int delta_time = 0;

    while(time>0){
        sleep(1);
        delta_time++;
        if(delta_time == freq){
            delta_time = 0;

            if(get_mod_date(file_name,&last_mod_date)!=0)  return copies;

            if(last_mod_date != mod_date){ //=> zmodyfikowano plik
                printf("Copying file: %s\n",file_name);
                if (mode == 0){
                    if (get_copy_name(file_name,mod_date,&copy_name)!=0) return copies;
                    if (save_copy(copy_name,text) != 0) return copies;
                }
                else{
                    pid_t pid = fork();
                    if (pid == 0){
                        if(get_copy_name(file_name,mod_date,&copy_name)!=0) return copies;
                        execlp("/bin/cp","cp",file_name,copy_name, NULL);
                    }
                }
                mod_date = last_mod_date;
                copies++;
            }
        }
        time --;
    }
    return copies;
}

int main(int argc, char ** argv) {

    if (argc != 6) {
        printf("Wrong number of args!\nInput: list time mode cpu_limit memory_limit.\n");
        return 1;
    }
    char *list = argv[1];
    FILE *file = fopen(list, "r");
    if(file == NULL){
        printf("Something went wrong with opening file: %s.\n",list);
        return 1;
    }

    if(is_a_number(argv[2])!=0 || is_a_number(argv[3])!=0 || is_a_number(argv[4])!=0 || is_a_number(argv[5])!=0){
        printf("2nd, 3rd, 4th & 5th arg should be nonnegative numbers.\n");
        return 1;
    }

    int time = atoi(argv[2]);
    mode = atoi(argv[3]);

    if (mode != 0 && mode != 1){
        printf("Wrong mode! Mode vals: 0 or 1.\n");
        return 1;
    }

    rlim_t cpu_limit = atoi(argv[4]);
    rlim_t memory_limit = atoi(argv[5]);

    fseek(file,0,SEEK_END);
    int list_size = ftell(file);
    fseek(file,0,SEEK_SET);

    char *paths = malloc(list_size*sizeof(char));
    int path_counter = 0;
    fread(paths, sizeof(char),list_size,file);
    char *current_path = strtok(paths, " \n");

    while (current_path!=NULL){
        path_counter++;
        int freq = atoi(strtok(NULL, " \n"));
        pid_t pid = fork();
        if (pid==0) return monitor (current_path,time,freq, cpu_limit, memory_limit);
        current_path = strtok(NULL," \n");
    }

    while(path_counter>0){
        int status;

        struct rusage before;
        struct rusage after;
        if(getrusage(RUSAGE_CHILDREN, &before) != 0) return 1;

        pid_t pid = wait(&status);

        if(getrusage(RUSAGE_CHILDREN, &after) != 0) return 1;

        if(pid >0) printf("\nPID %d created %d copies.\n",pid, WEXITSTATUS(status));
        else{
            printf("Something went wrong with changing process status.\n");
            return 1;
        }
        printf("User time: %ld\n",after.ru_utime.tv_usec - before.ru_utime.tv_usec);
        printf("System time: %ld\n\n",after.ru_stime.tv_usec - before.ru_stime.tv_usec);
        path_counter--;
    }

    return 0;
}
