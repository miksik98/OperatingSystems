#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <memory.h>
#include <ftw.h>
#include <limits.h>
#include <sys/types.h>

time_t date;
int comp;

void print_file_stats(const struct stat * file_stat,const char * path){

    printf("Absolute path: %s\n", path);
    printf("File type: ");
    switch (file_stat->st_mode & S_IFMT) {
        case S_IFBLK:  printf("block dev\n");               break;
        case S_IFCHR:  printf("char dev\n");                break;
        case S_IFDIR:  printf("dir\n");                     break;
        case S_IFIFO:  printf("fifo\n");                    break;
        case S_IFLNK:  printf("slink\n");                   break;
        case S_IFREG:  printf("file\n");                    break;
        case S_IFSOCK: printf("sock\n");                    break;
        default:       printf("unknown?\n");                break;
    }
    printf("File size in bytes: %li \n", file_stat->st_size);
    printf("Last modified: %s", ctime(&file_stat->st_mtime));
    printf("Last access: %s \n", ctime(&file_stat->st_atime));

}

int is_a_number(char * string){
    for (int i = 0; i < strlen(string); i++){
        if((((int)string[i]>57) || ((int)string[i]<48))) return 1;
    }
    return 0;
}

int data_parser(char * year, char * mon, char * day, char * hour, char * min, char * sec){
    if (is_a_number(year)==1 || is_a_number(mon)==1 || is_a_number(day)==1 || is_a_number(hour)==1 || is_a_number(min)==1 || is_a_number(sec)==1) return 1;
    if (atoi(year)<1900){
        printf("Wrong year.\n");
        return 1;
    }
    if (atoi(mon)<1 || atoi(mon)>12){
        printf("Wrong month.\n");
        return 1;
    }
    if (atoi(day)<1 || atoi(day)>31){
        printf("Wrong day.\n");
        return 1;
    }
    if (atoi(hour)<0 || atoi(hour)>23){
        printf("Wrong hour.\n");
        return 1;
    }
    if (atoi(min)<0 || atoi(min)>59){
        printf("Wrong minute.\n");
        return 1;
    }
    if (atoi(sec)<0 || atoi(sec)>59){
        printf("Wrong second.\n");
        return 1;
    }
    return 0;
}

int get_int_comp(char * operator){
    if(strcmp(operator,">")==0) return -1;
    if(strcmp(operator,"=")==0) return 0;
    if(strcmp(operator,"<")==0) return 1;
    return 2;
}

int explore_directory(const char * path, const struct stat * file_stat, int file_flag, struct FTW *ftw){
    if (file_flag==FTW_F){
        if((comp == -1 && date < file_stat->st_mtime) ||
           (comp == 0 && date == file_stat->st_mtime) ||
           (comp == 1 && date > file_stat->st_mtime))
            print_file_stats(file_stat,path);
    }
    return 0;
}


int main(int argc, char ** argv) {

    if (argc != 9) {
        printf("Wrong number of args. See Readme.\n");
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
    comp = get_int_comp(argv[2]);
    if (comp==2){
        printf("Wrong operator. There are only 3 operators: \'>\',\'=\',\'<\'.\n");
        return 1;
    }

    if(data_parser(argv[3],argv[4],argv[5],argv[6],argv[7],argv[8])==1){
        printf("Remember about data args: year month day hour minute second\n");
        return 1;
    }

    struct tm tm_time;
    tm_time.tm_year = atoi(argv[3]) - 1900;
    tm_time.tm_mon = atoi(argv[4]) - 1;
    tm_time.tm_mday = atoi(argv[5]);
    tm_time.tm_hour = atoi(argv[6]);
    tm_time.tm_min = atoi(argv[7]);
    tm_time.tm_sec = atoi(argv[8]);
    tm_time.tm_isdst = 0;
    date = mktime(&tm_time);

    nftw(path, explore_directory, 20, FTW_PHYS);


    return 0;
}