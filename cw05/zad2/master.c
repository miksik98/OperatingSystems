#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

// ./master potok

int main(int argc, char *argv[]) {

    if(argc != 2) {
        printf("Wrong number of args! Just put fifo_path name.\n");
        return -1;
    }

    if(mkfifo(argv[1], 0600) < 0) { //0600 := r+w
        printf("Sth went wrong.\n");
        return -1;
    }

    int fifo;
    if((fifo = open(argv[1], O_RDONLY)) < 0) {
        printf("Could not open a file!\n");
        return -1;
    }

    char *data_from_fifo = calloc(64, sizeof(char));
    while(1) {
        if(read(fifo, data_from_fifo, 64)) printf("%s", data_from_fifo);
    }

    free(data_from_fifo);
    close(fifo);
    return 0;
}