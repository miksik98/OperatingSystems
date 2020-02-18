#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <dlfcn.h>

void * handle;

//wskaźniki do funkcji z biblioteki dynamicznej
struct block_arr *(*create_array)(int);
void (*delete_block_at_index)(struct block_arr *,int);
int (*reserve_block)(struct block_arr *, char*);
void (*search_file)(char *);
void (*set_dir_file)(char*,char*);
void(*prepare_dir_file)(void);

double calculate_time(clock_t start, clock_t end){
    return (double)(end -  start) / sysconf(_SC_CLK_TCK);
}

void printTime(clock_t rStartTime, struct tms tmsStartTime, clock_t rEndTime, struct tms tmsEndTime){
    printf("Real:   %lf s   ", calculate_time(rStartTime, rEndTime));
    printf("User:   %lf s   ", calculate_time(tmsStartTime.tms_utime+tmsStartTime.tms_cutime, tmsEndTime.tms_utime+tmsEndTime.tms_cutime));
    printf("System: %lf s\n", calculate_time(tmsStartTime.tms_stime+tmsStartTime.tms_cstime, tmsEndTime.tms_stime+tmsEndTime.tms_cstime));
}

int is_a_number(char * string){
    for (int i = 0; i < strlen(string); i++){
        if (((int)string[i]>57) || ((int)string[i]<48)) return 0;
    }
    return 1;
}

void load_dynamic_library(){
    handle = dlopen("./library.so", RTLD_LAZY);
    if(handle == NULL)
    {
        fprintf(stderr, "error occured while loading library:\n");
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    //wypełnianie wskaźników do funkcji
    create_array = (struct block_arr * (*) (int))dlsym(handle,"create_array");
    delete_block_at_index = (void (*) (struct block_arr *,int))dlsym(handle,"delete_block_at_index");
    reserve_block = (int (*) (struct block_arr *,char*))dlsym(handle,"reserve_block");
    search_file = (void (*) (char*))dlsym(handle,"search_file");
    set_dir_file = (void (*) (char*,char*))dlsym(handle,"set_dir_file");
    prepare_dir_file = (void (*) (void))dlsym(handle, "prepare_dir_file");

    if (dlerror()!=NULL){
        printf("Cannot load library functions\n");
        exit(1);
    }
}
void test_searching(char * dir_big, char * dir_medium, char * dir_small, char * file){
    load_dynamic_library();
    struct tms **tms_time = malloc(7 * sizeof(struct tms *));
    clock_t real_time[6];
    for (int i = 0; i < 7; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms *));
    }

    real_time[0] = times(tms_time[0]);

    set_dir_file(dir_big,file);
    search_file("result");

    real_time[1] = times(tms_time[1]);

    real_time[2] = times(tms_time[2]);

    set_dir_file(dir_medium,file);
    search_file("result");

    real_time[3] = times(tms_time[3]);

    real_time[4] = times(tms_time[4]);

    set_dir_file(dir_small,file);
    search_file("result");

    real_time[5] = times(tms_time[5]);

    printf("%s", "searching in deep\n");
    printTime(real_time[0],*tms_time[0], real_time[1], *tms_time[1]);
    printf("%s", "searching in middle-deep\n");
    printTime(real_time[2],*tms_time[2], real_time[3], *tms_time[3]);
    printf("%s", "searching in not deep\n");
    printTime(real_time[4],*tms_time[4], real_time[5], *tms_time[5]);
    system("rm result");
    dlclose(handle);
}

void test_add_remove(char * big_file, char * medium_file, char * small_file, struct block_arr * arr){
    load_dynamic_library();
    struct tms * tms_time_start = (struct tms *) malloc(sizeof(struct tms *));
    struct tms * tms_time_end = (struct tms *) malloc(sizeof(struct tms *));
    clock_t real_time_start, real_time_end;
    //arr = create_array(1000);
    real_time_start = times(tms_time_start);
    for (int i = 0; i<1000000; i++){
        if (i%2 == 0){
            reserve_block(arr,big_file);
        }
        if (i%4 == 1){
            reserve_block(arr,medium_file);
        }
        if (i%4 == 3){
            reserve_block(arr,small_file);
        }
    }
    real_time_end = times(tms_time_end);
    printf("%s", "reserving_blocks\n");
    printTime(real_time_start,*tms_time_start, real_time_end, *tms_time_end);

    real_time_start = times(tms_time_start);
    for (int i = 0; i<1000; i++){
        delete_block_at_index(arr,i);
    }
    real_time_end = times(tms_time_end);

    printf("%s", "deleting_blocks\n");
    printTime(real_time_start,*tms_time_start, real_time_end, *tms_time_end);
    dlclose(handle);
}

void test_ar_loop(int number_of_blocks, char * file, struct block_arr * arr){
    load_dynamic_library();
    struct tms * tms_time_start = (struct tms *) malloc(sizeof(struct tms *));
    struct tms * tms_time_end = (struct tms *) malloc(sizeof(struct tms *));
    clock_t real_time_start, real_time_end;
    //arr = create_array(1000);
    real_time_start = times(tms_time_start);
    for (int j = 0; j<10; j++) {
        for (int i = 0; i < number_of_blocks; i++) {
            reserve_block(arr, file);
        }
        for (int i = 0; i < number_of_blocks; i++) {
            delete_block_at_index(arr, i);
        }

    }



    real_time_end = times(tms_time_end);
            printf("%s", "adding and deleting_blocks\n");
            printTime(real_time_start,*tms_time_start, real_time_end, *tms_time_end);
            dlclose(handle);
    }



int main(int argc, char **argv) {

    load_dynamic_library();
    prepare_dir_file();
    struct block_arr * arr =  create_array(0);
    struct block_arr * test_arr = create_array(1000);

    //test_ar_loop(100,"c.txt",arr);}

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "create_table") == 0) {
            if (i == argc - 1) {
                printf("Too few arguments! When you use "
                       "create_table just add second parameter of its size.\n");
                return 1;
            }
            i++;
            if (is_a_number(argv[i]) == 0) {
                printf("The second arg in create_table function should be a nonnegative number.\n");
                return 1;
            }
            int number_of_blocks = atoi(argv[i]);
            arr = create_array(number_of_blocks);
        } else {
            if (strcmp(argv[i], "search_directory") == 0) {
                char * command = "Too few arguments! When you use search_directory just add parameters: dir \"file\" \"name_file_temp\".\n";
                if (i == argc - 1) {
                    printf(command);
                    return 1;
                }
                i++;
                char *dir = malloc(strlen(argv[i]) * sizeof(char));
                strcpy(dir, argv[i]);
                if (i == argc - 1) {
                    printf(command);
                    return 1;
                }
                i++;
                char *file = malloc(strlen(argv[i]) * sizeof(char));
                strcpy(file, argv[i]);
                printf("%s ", file);
                if (i == argc - 1) {
                    printf(command);
                    return 1;
                }
                i++;
                char *name_file_temp = malloc(strlen(argv[i]) * sizeof(char));
                strcpy(name_file_temp, argv[i]);
                set_dir_file(dir, file);
                search_file(name_file_temp);
                reserve_block(arr, name_file_temp);
            } else {
                if (strcmp(argv[i], "remove_block") == 0) {
                    if (i == argc - 1) {
                        printf("Too few arguments! When you use "
                               "remove_block just add parameters: dir \"file\" \"name_file_temp.\"\n");
                        return 1;
                    }
                    i++;
                    if (is_a_number(argv[i]) == 0) {
                        printf("The second arg in create_table function should be a nonnegative number.\n");
                        return 1;
                    }
                    
		   // delete_block_at_index(arr, atoi(argv[i]));

                } else {
                    if (strcmp(argv[i], "test_searching") == 0){
                        char * command = "Too few arguments! When you use "
                                         "test_searching just add parameters: big_dir medium_dir small_dir \"file\"\n";
                        if (i == argc - 1) {
                            printf(command);
                            return 1;
                        }
                        i++;
                        char *big_dir = malloc(strlen(argv[i]) * sizeof(char));
                        strcpy(big_dir, argv[i]);
                        if (i == argc - 1) {
                            printf(command);
                            return 1;
                        }
                        i++;
                        char *medium_dir = malloc(strlen(argv[i]) * sizeof(char));
                        strcpy(medium_dir, argv[i]);
                        if (i == argc - 1) {
                            printf(command);
                            return 1;
                        }
                        i++;
                        char *small_dir = malloc(strlen(argv[i]) * sizeof(char));
                        strcpy(small_dir, argv[i]);
                        if (i == argc - 1) {
                            printf(command);
                            return 1;
                        }
                        i++;

                        test_searching(big_dir,medium_dir,small_dir,argv[i]);
                    }
                    else {
                        if (strcmp(argv[i], "test_add_remove") == 0){
                            char * command = "Too few arguments! When you use "
                                             "test_add_remove just add parameters: big_file_block medium_file_block small_file_block\n";
                            if (i == argc - 1) {
                                printf(command);
                                return 1;
                            }
                            i++;
                            char *big_file = malloc(strlen(argv[i]) * sizeof(char));
                            strcpy(big_file, argv[i]);
                            if (i == argc - 1) {
                                printf(command);
                                return 1;
                            }
                            i++;
                            char *medium_file = malloc(strlen(argv[i]) * sizeof(char));
                            strcpy(medium_file, argv[i]);
                            if (i == argc - 1) {
                                printf(command);
                                return 1;
                            }
                            i++;

                            test_add_remove(big_file,medium_file,argv[i],test_arr);
                        }
                        else
                        {
                            if (strcmp(argv[i], "test_ar_loop") == 0){
                                char * command = "Too few arguments! When you use "
                                                 "test_ar_loopjust add parameters: number_of_blocks file_block\n";
                                if (i == argc - 1) {
                                    printf(command);
                                    return 1;
                                }
                                i++;
                                char *number_of_blocks = malloc(strlen(argv[i]) * sizeof(char));
                                strcpy(number_of_blocks, argv[i]);
                                if (i == argc - 1) {
                                    printf(command);
                                    return 1;
                                }
                                i++;
                                if (is_a_number(number_of_blocks) == 0) {
                                    printf("The first arg in test_ar_loop function should be a nonnegative number.\n");
                                    return 1;
                                }
                                test_ar_loop(atoi(number_of_blocks),argv[i],test_arr);
                            }
                            else{
                                printf("Incorrect command: ");
                                printf("%s\n", argv[i]);
                                break;
                            }
                        }
                    }

                }
            }
        }

    }
    dlclose(handle);
}

