#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// ./slave potok 20


int is_a_number(char * string){
    for (int i=0; i< (int)strlen(string);i++){
        if((int)string[i]>57 || ((int)string[i]<48)) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    srand(time(0));

    if(argc != 3) {
        printf("Wrong number of args! Input: <fifo_path> <number_of_lines>.\n");
        return -1;
    }

    int fifo;

    if((fifo = open(argv[1], O_WRONLY)) < 0) {
        printf("Could not open a file!\n");
        return -1;
    }

    printf("%d\n", getpid());

    if (is_a_number(argv[2]) == 1){
        printf("2nd arg should be a nonnegative number.\n");
        return -1;
    }
    int n = atoi(argv[2]);

    while(n > 0) {
        FILE *date;
        if((date = popen("date", "r")) == NULL) {
            printf("Could not open a file.\n");
            return -1;
        }

        char *line = calloc(64, sizeof(char));
        sprintf(line, "%d ", getpid());
        char temp[64];
        fread(temp, sizeof(char), 64, date);
        strcat(line, temp);
        write(fifo, line, 64*sizeof(char));

        pclose(date);

        free(line);

        int rand_s = rand() % 4 + 2; // 2-5 seconds

        if(n != 0) sleep(rand_s);

        n--;
    }
    close(fifo);

    return 0;
}