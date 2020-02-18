#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/stat.h>

double diff_time(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

void print_time(struct tms *tms_start_time, struct tms *tms_end_time){
    printf("User: %lf s ",diff_time(tms_start_time->tms_utime+tms_start_time->tms_cutime,tms_end_time->tms_utime+tms_end_time->tms_cutime));
    printf("System: %lf s\n",diff_time(tms_start_time->tms_stime+tms_start_time->tms_cstime,tms_end_time->tms_stime+tms_end_time->tms_cstime));
}

int is_a_number(char * string){
    for (int i = 0; i < (int)strlen(string); i++){
        if((int)string[i]>57 || ((int)string[i]<48)) return 1;
    }
    return 0;
}

int generate(char * file_name, int records, int length){
    srand(time(NULL));
    FILE *file = fopen(file_name, "w+");
    char *char_set = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *tmp = malloc((length+1)*sizeof(char*));
    for (int i = 0; i < records; i++){
        int key = 0;
        for (int j = 0; j < length; j++){
            key = rand()%((int)strlen(char_set));
            tmp[j] = char_set[key];
        }
        tmp[length] = '\0';

        if(fwrite(tmp,sizeof(char),(size_t) length + 1,file) != length+1){
            return 1;
        }
    }
    fclose(file);
    free(tmp);
    return 0;
}

int library_sort(char * file_name, int records, int length){
    FILE *file = fopen(file_name,"r+");
    char *record1 = malloc((length+1)*sizeof(char));
    char *record2 = malloc((length+1)*sizeof(char));

    int offset = (int) ((length+1)*sizeof(char));

    for (int i = 0; i < records; i++){
        fseek(file, i * offset, 0);
        if (fread(record1, sizeof(char), (size_t)(length + 1), file)!=length+1) return 1;

        for (int j = 0; j < i; j++){
            fseek(file, j * offset, 0);
            if (fread(record2, sizeof(char), (size_t)(length+1),file)!=length+1) return 1;
            if (record2[0]>record1[0]){
                fseek(file, j * offset, 0);
                if (fwrite(record1, sizeof(char), (size_t)(length+1), file)!=length+1) return 1;
                fseek(file, i * offset, 0);
                if (fwrite(record2, sizeof(char), (size_t)(length+1),file)!=(length+1)) return 1;
                char *tmp = record1;
                record1 = record2;
                record2 = tmp;
            }
        }
    }
    fclose(file);
    free(record1);
    free(record2);
    return 0;

}

int library_copy(char *file1, char *file2, int records, int length){
    FILE *source = fopen(file1,"r");
    FILE *dest = fopen(file2,"w+");
    char *tmp = malloc(length * sizeof(char));
    for (int i = 0; i < records; i++){
        if(fread(tmp, sizeof(char), (size_t) (length + 1), source) != (length + 1)) {
            return 1;
        }

        if(fwrite(tmp, sizeof(char), (size_t) (length + 1), dest) != (length + 1)) {
            return 1;
        }
    }
    fclose(source);
    fclose(dest);
    free(tmp);
    return 0;
}

int system_sort(char * file_name, int records, int length){
    int file = open(file_name, O_RDWR);
    char *record1 = malloc((length+1)*sizeof(char));
    char *record2 = malloc((length+1)*sizeof(char));

    int offset = (int) ((length+1)*sizeof(char));

    for (int i = 0; i < records; i++){
        lseek(file, i * offset, SEEK_SET);
        if (read(file, record1, (size_t)(length+1)*sizeof(char))!=length+1) return 1;

        for (int j = 0; j < i; j++){
            lseek(file, j * offset, SEEK_SET);
            if (read(file, record2, (size_t)(length+1)*sizeof(char))!=length+1) return 1;
            if (record2[0]>record1[0]){
                lseek(file, j * offset, SEEK_SET);
                if (write(file, record1, (size_t)(length+1)*sizeof(char))!=length+1) return 1;
                lseek(file, i * offset, SEEK_SET);
                if (write(file, record2, (size_t)(length+1)*sizeof(char))!=length+1) return 1;
                char *tmp = record1;
                record1 = record2;
                record2 = tmp;
            }
        }
    }
    close(file);
    free(record1);
    free(record2);
    return 0;

}

int system_copy(char *file1, char *file2, int records, int length){
    int source = open(file1, O_RDONLY);
    int dest = open(file2, O_CREAT | O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR);
    char *tmp = malloc(length * sizeof(char));
    for (int i = 0; i < records; i++){
        if(read(source, tmp, (size_t) (length + 1) * sizeof(char)) != (length + 1)) {
            return 1;
        }

        if(write(dest, tmp, (size_t) (length + 1) * sizeof(char)) != (length+ 1)) {
            return 1;
        }
    }
    close(source);
    close(dest);
    free(tmp);
    return 0;
}

int test_system_sort(char * file_name, int records, int length){

    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    //clock_t real_time[2];
    for (int i = 0; i < 2; i++) {
        tms_time[i] = (struct tms *) malloc(4*sizeof(struct tms *));
    }
    times(tms_time[0]);
    if (system_sort(file_name,records,length) == 1){
        return 1;
    }
    times(tms_time[1]);
    printf("System sort test\n");
    print_time(tms_time[0],tms_time[1]);
    return 0;
}

int test_library_sort(char * file_name, int records, int length){
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    //clock_t real_time[2];
    for (int i = 0; i < 2; i++) {
        tms_time[i] = (struct tms *) malloc(4*sizeof(struct tms *));
    }
    times(tms_time[0]);
    if (library_sort(file_name,records,length) == 1){
        return 1;
    }
    times(tms_time[1]);
    printf("Library sort test\n");
    print_time(tms_time[0],tms_time[1]);
    return 0;
}

int test_system_copy(char *file1, char *file2, int records, int length){
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    //clock_t real_time[2];
    for (int i = 0; i < 2; i++) {
        tms_time[i] = (struct tms *) malloc(4*sizeof(struct tms *));
    }
    times(tms_time[0]);
    if (system_copy(file1,file2,records,length) == 1){
        return 1;
    }
    times(tms_time[1]);
    printf("System copy test\n");
    print_time(tms_time[0],tms_time[1]);
    return 0;
}

int test_library_copy(char *file1, char *file2, int records, int length){
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    //clock_t real_time[2];
    for (int i = 0; i < 2; i++) {
        tms_time[i] = (struct tms *) malloc(4 *sizeof(struct tms *));
    }
    times(tms_time[0]);
    if (library_copy(file1,file2,records,length) == 1){
        return 1;
    }
    times(tms_time[1]);
    printf("Library copy test\n");
    print_time(tms_time[0],tms_time[1]);
    return 0;
}
int main(int argc, char **argv) {

    char *file_name;
    char *fun_type;
    int records,length;
    switch(argc){
        case 5:
            if(strcmp(argv[1],"generate") != 0){
                printf("Wrong command. See Readme.\n");
                return 1;
            }
            file_name = argv[2];
            if(is_a_number(argv[3]) == 1 || is_a_number(argv[4]) == 1){
                printf("3rd & 4th argument of \"generate\" fun should be nonnegative numbers.\n");
                return 1;
            }
            records = strtol(argv[3],NULL,10);
            length = strtol(argv[4],NULL,10);
            if (generate(file_name,records,length) == 1){
                printf("Something went wrong with generating.\n");
            }
            break;
        case 6:
            if(strcmp(argv[1],"sort") != 0){
                printf("Wrong command. See Readme.\n");
                return 1;
            }
            file_name = argv[2];
            if(is_a_number(argv[3]) == 1 || is_a_number(argv[4]) == 1){
                printf("3rd & 4th argument of \"sort\" fun should be nonnegative numbers.\n");
                return 1;
            }
            records = strtol(argv[3],NULL,10);
            length = strtol(argv[4],NULL,10);
            fun_type = argv[5];
            if(strcmp(fun_type,"sys") == 0){
                if(system_sort(file_name,records,length) == 1){
                    printf("Something went wrong with sorting.\n");
                    return 1;
                }
            }
            else{
                if(strcmp(fun_type,"lib") == 0){
                    if(library_sort(file_name,records,length) == 1){
                        printf("Something went wrong with sorting.\n");
                        return 1;
                    }
                }
                else{
                    printf("5th arg of \"sort\" fun should be \"sys\" or \"lib\" string. Check Readme.\n");
                    return 1;
                }
            }

            break;
        case 7:
            if(strcmp(argv[1],"test") == 0){
                if(strcmp(argv[2],"sort") != 0){
                    printf("Wrong command. See Readme.\n");
                    return 1;
                }
                file_name = argv[3];
                if(is_a_number(argv[4]) == 1 || is_a_number(argv[5]) == 1){
                    printf("3rd & 4th argument of \"sort\" fun should be nonnegative numbers.\n");
                    return 1;
                }
                records = strtol(argv[4],NULL,10);
                length = strtol(argv[5],NULL,10);
                fun_type = argv[6];
                if(strcmp(fun_type,"sys") == 0){
                    if(test_system_sort(file_name,records,length) == 1){
                        printf("Something went wrong with test sorting.\n");
                        return 1;
                    }
                }
                else{
                    if(strcmp(fun_type,"lib") == 0){
                        if(test_library_sort(file_name,records,length) == 1){
                            printf("Something went wrong with test sorting.\n");
                            return 1;
                        }
                    }
                    else{
                        printf("5th arg of \"sort\" fun should be \"sys\" or \"lib\" string. Check Readme.\n");
                        return 1;
                    }
                }
            }
            else {
                if (strcmp(argv[1], "copy") == 0) {
                    file_name = argv[2];
                    char *destination = argv[3];
                    if (is_a_number(argv[4]) == 1 || is_a_number(argv[5]) == 1) {
                        printf("4th & 5th argument of \"copy\" fun should be nonnegative numbers.\n");
                        return 1;
                    }
                    records = strtol(argv[4], NULL, 10);
                    length = strtol(argv[5], NULL, 10);
                    fun_type = argv[6];
                    if (strcmp(fun_type, "sys") == 0) {
                        if (system_copy(file_name, destination, records, length) == 1) {
                            printf("Something went wrong with copying.\n");
                            return 1;
                        }
                    } else {
                        if (strcmp(fun_type, "lib") == 0) {
                            if (library_copy(file_name, destination, records, length) == 1) {
                                printf("Something went wrong with copying.\n");
                                return 1;
                            }
                        } else {
                            printf("5th arg of \"copy\" fun should be \"sys\" or \"lib\" string. Check Readme.\n");
                            return 1;
                        }
                    }
                }
                else{
                    printf("Wrong command. See Readme.\n");
                    return 1;
                }
            }
                break;
        case 8:
            if (strcmp(argv[1],"test") == 0) {
                if (strcmp(argv[2], "copy") == 0) {
                    file_name = argv[3];
                    char *destination = argv[4];
                    if (is_a_number(argv[5]) == 1 || is_a_number(argv[6]) == 1) {
                        printf("4th & 5th argument of \"copy\" fun should be nonnegative numbers.\n");
                        return 1;
                    }
                    records = strtol(argv[5], NULL, 10);
                    length = strtol(argv[6], NULL, 10);
                    fun_type = argv[7];
                    if (strcmp(fun_type, "sys") == 0) {
                        if (test_system_copy(file_name, destination, records, length) == 1) {
                            printf("Something went wrong with test copying.\n");
                            return 1;
                        }
                    } else {
                        if (strcmp(fun_type, "lib") == 0) {
                            if (test_library_copy(file_name, destination, records, length) == 1) {
                                printf("Something went wrong with test copying.\n");
                                return 1;
                            }
                        } else {
                            printf("5th arg of \"copy\" fun should be \"sys\" or \"lib\" string. Check Readme.\n");
                            return 1;
                        }
                    }
                } else {
                    printf("Wrong command. See Readme.\n");
                    return 1;
                }
            }
            else{
                printf("Wrong command. See Readme.\n");
                return 1;
            }
            break;
        default:
            printf("Wrong command. See Readme.\n");
            break;
    }
    return 0;
}