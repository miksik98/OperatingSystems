#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

int **image;
int image_height;
int image_width;

double **filter;
int filter_size;

int **result;

int thread_count;

int is_a_number(char * string){
    for (int i = 0; i < strlen(string); i++){
        if (((int)string[i]>57)||((int)string[i]<48)) return 0; // not digit
    }
    return 1;
}

int max(int a, int b){
    return a > b ? a : b;
}


double splot(int x, int y){
    double sum = 0;
    for(int i = 0; i < filter_size; i++){
        for(int j = 0; j < filter_size; j++){
            int height = max(0, x - ceil(filter_size/2.0) + i);
            int weight = max(0, y - ceil(filter_size/2.0) + j);
            if(height < image_height && weight < image_width)
                sum += (image[height][weight] * filter[i][j]);
        }
    }
    return round(sum);
}

struct timeval *block_filter(void *args){
    int *arg = (int*) args;
    int i = arg[0];

    struct timeval *start = malloc(sizeof(struct timeval));
    struct timeval *end = malloc(sizeof(struct timeval));
    struct timeval *res = malloc(sizeof(struct timeval));

    gettimeofday(start,NULL);

    int x_start = ceil(i * image_width / (1.0*thread_count));
    int x_end = ceil((i + 1) * image_width / (1.0*thread_count)) - 1;
    for(int p = x_start; p <= x_end; p++){
        for(int r = 0; r < image_height; r++){
            result[r][p] = abs(splot(r, p));
        }
    }

    gettimeofday(end,NULL);
    timersub(end,start,res);

    free(start);
    free(end);
    return res;
}

struct timeval *interleaved_filter(void *args){
    int *arg = (int*) args;
    int i = arg[0];

    struct timeval *start = malloc(sizeof(struct timeval));
    struct timeval *end = malloc(sizeof(struct timeval));
    struct timeval *res = malloc(sizeof(struct timeval));

    gettimeofday(start,NULL);

    for(int p = i; p < image_width; p += thread_count){
        for(int r = 0; r < image_height; r++){
            result[r][p] = abs(splot(r, p));
        }
    }

    gettimeofday(end,NULL);
    timersub(end,start,res);
    free(start);
    free(end);

    return res;
}


int load_image(FILE* file){
    int max_color;

    if(getc(file) != 'P') return 1;
    else if (getc(file) != '2') return 1;
    while(getc(file) != '\n');

    while(getc(file) == '#'){
        while(getc(file) != '\n');
    }

    fseek(file, -1, SEEK_CUR);

    if(fscanf(file, "%d", &image_width) != 1) return 1;
    if(fscanf(file, "%d", &image_height) != 1) return 1;
    if(fscanf(file, "%d", &max_color) != 1) return 1;
    if(max_color > 255) return 1;

    image = malloc(image_height * sizeof(char*));
    result = malloc(image_height * sizeof(char*));

    for(int i = 0; i < image_height; i++) {
        image[i] = malloc(image_width * sizeof(int));
        result[i] = malloc(image_width * sizeof(int));
    }

    for(int i = image_height - 1; i >= 0; i--){
        for(int j = 0 ; j < image_width; j++){
            if(fscanf(file, "%d", &image[i][j]) != 1) return 1;
        }
    }
    return 0;
}

int load_filter(FILE* file){
    if(fscanf(file, "%d", &filter_size) != 1) return 1;

    filter = malloc(filter_size*sizeof(double*));
    for(int i = 0; i < filter_size; i++) filter[i] = malloc(filter_size*sizeof(double));

    for(int i = 0; i < filter_size; i++){
        for(int j = 0; j < filter_size; j++){
            if(fscanf(file, "%lf", &filter[i][j]) != 1) return 1;
        }
    }
    return 0;
}

int save_result(FILE* file){
    fprintf(file,"P2\n%d %d\n%d\n",image_width,image_height,255);
    for(int i = image_height - 1; i >= 0; i--){
        for(int j = 0 ; j < image_width; j++){
            if(j % 10 == 0) fprintf(file,"\n");
            fprintf(file, "%d ", result[i][j]);
        }
    }
    return 0;
}

void cleaner(){
    if(image != NULL){
        for(int i = 0; i < image_height; i++) free(image[i]);
        free(image);
    }

    if(filter != NULL){
        for(int i = 0; i < filter_size; i++) free(filter[i]);
        free(filter);
    }

    if(result != NULL){
        for(int i = 0; i < image_height; i++) free(result[i]);
        free(result);
    }
}

int main(int argc, char **argv){
    image = NULL;
    filter = NULL;
    result = NULL; //if error => cleaner will free memory
    int *threads;
    if(argc != 6){
        printf("Wrong number of args.\n Correct input: threads_number mode input_file filter output_file\n");
        exit(1);
    }
    FILE *file = fopen(argv[3],"r");
    if(file == NULL) {
        printf("Error with opening input file\n");
        exit(1);
    }
    if(!is_a_number(argv[1])){
        printf("First arg should be a nonnegative number\n");
        exit(1);
    }

    thread_count = atoi(argv[1]);
    threads = malloc(thread_count*sizeof(int));
    for(int i = 0; i < thread_count; i++) threads[i] = i;

    if(load_image(file) != 0) {
        printf("Wrong format of file!\n");
        exit(1);
    }
    fclose(file);

    file = fopen(argv[4],"r");
    if(file == NULL) {
        printf("Error with opening filter file\n");
        exit(1);
    }

    if(load_filter(file) != 0) {
        printf("Wrong format of file!\n");
        exit(1);
    }
    fclose(file);
    atexit(cleaner);

    pthread_t tids[thread_count];
    if(strcmp(argv[2],"block") == 0){
        for(int i = 0; i < thread_count; i++){
            if(pthread_create(&tids[i],NULL,(void*)(void*)block_filter,&threads[i]) !=0) {
                printf("Error with creating thread\n");
                exit(1);
            }
        }
    }
    else if(strcmp(argv[2],"interleaved") == 0){
        for(int i = 0; i < thread_count; i++){
            if(pthread_create(&tids[i],NULL,(void*)(void*)interleaved_filter,&threads[i]) != 0) {
                printf("Error with creating thread\n");
                exit(1);
            }
        }
    }
    else {
        printf("Wrong mode! Choose one: block / interleaved\n");
        exit(1);
    }

    for(int i = 0; i < thread_count; i++){
        struct timeval *work_time;
        if(pthread_join(tids[i],(void*)&work_time) != 0) {
            printf("Error with pthread_join\n");
            exit(1);
        }
        else{
            printf("Thread %ld worked %ld.%.6ld seconds \n",tids[i],work_time->tv_sec,work_time->tv_usec);
        }
    }

    file = fopen(argv[5],"w");
    if(file == NULL) {
        printf("Error with opening output file\n");
        exit(1);
    }
    if(save_result(file) != 0) {
        printf("Sth went wrong with saving result\n");
        exit(1);
    }
    fclose(file);

    return 0;
}

