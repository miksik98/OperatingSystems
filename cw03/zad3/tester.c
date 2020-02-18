#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

int is_a_number(char * string){
    for (int i = 0; i < strlen(string); i++){
        if (((int)string[i]>57)||((int)string[i]<48)) return 1; // not digit
    }
    return 0;
}

char * generate_string(int bytes){
    srand(time(NULL));
    char *char_set = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    char *result = malloc((bytes+1)*sizeof(char));
    int key = 0;
    for (int i = 0; i<bytes;i++){
        key = rand()%((int)strlen(char_set));
        result[i] = char_set[key];
    }
    result[bytes] = '\0';
    return result;
}

int main(int argc, char **argv) {

    if(argc != 5) {
        printf("Wrong number of args!\nCorrect input: file pmin pmax bytes\n");
        return 1;
    }

    if (is_a_number(argv[2])!=0 || is_a_number(argv[3])!=0 || is_a_number(argv[4])!=0){
        printf("2nd, 3rd & 4th arg should be nonnegative numbers.\n");
        return 1;
    }

    char *filename = argv[1];
    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);

    srand(time(NULL));
    pid_t pid = fork();

    if(pid == 0) {
        int sleep_time = rand() % (pmax - pmin + 1) + pmin; // min: 0+pmin=pmin // max: pmax-pmin+pmin=pmax
        char *date = malloc(21 * sizeof(char));

        int duration = 45;
        while(duration > 0) {
            sleep(sleep_time);
            FILE *file = fopen(filename, "a");
            if(file == NULL) {
                printf("Sth went wrong with opening file.\n");
                return 1;
            }
            time_t current_time = time(NULL);
            char *random = generate_string(bytes);
            strftime(date, 21, "%Y-%m-%d_%H-%M-%S", localtime(&current_time));
            fprintf(file, "pid: %d seconds: %d date: %s random string: %s\n", getpid(), sleep_time, date, random);
            fclose(file);
            duration -= sleep_time;
        }
    }
    return 0;
}