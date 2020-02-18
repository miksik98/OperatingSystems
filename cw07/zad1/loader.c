#include "conveyor_belt.h"

int weight_of_product;

void prepare_memory(){
    key_t key = ftok(".",PROJECT_ID);
    shm_id = shmget(key,0,0);
    if(shm_id==-1)
    {
        printf("Error with getting shared memory\n");
        exit(1);
    }
    sem_id = semget(key,0,0);
    queue = shmat(shm_id,NULL,0);
}

int main(int argc, char **argv){
    if(argc!=2 && argc!=3)
    {
        printf("Bad number of arguments. See Readme.txt.\n");
        exit(1);
    }

    int cycles = 1;
    weight_of_product = atoi(argv[1]);

    if(argc==3)
    {
        cycles = atoi(argv[2]);
    }

    prepare_memory();

    struct product product;
    product.val = weight_of_product;
    struct sembuf sembuf;
    sembuf.sem_num = 0;
    sembuf.sem_op = 1;
    time_t rawtime;
    int packages_loaded = 0;

    while(packages_loaded < cycles){
        if(!is_full(queue)){
            time (&rawtime);
            printf("Time: %ld ",rawtime);
            product.date_add_to_queue = rawtime;
            product.loader_id = getpid();
            if(insert(product,queue)){
                printf("Insert to queue | weight: %d | loader id: %d\n", weight_of_product, getpid());
                if(argc==3){
                    packages_loaded++;
                }
            }
            else{
                printf("Package too heavy | weight: %d | weight left: %d\n", product.val, queue->max_weight-queue->current_weight);
            }
        }
        else{
            time (&rawtime);
            printf("Time: %ld ",rawtime);
            printf("Queue is full - waiting for space | loader id: %d\n", getpid());
        }
        sembuf.sem_op=1;
        semop(sem_id,&sembuf,1);
        sembuf.sem_op=-1;
        semop(sem_id,&sembuf,1);
    }
    printf("Loader with id: %d finished. Loaded %d packages\n", getpid(), cycles);
    sembuf.sem_op=1;
    semop(sem_id,&sembuf,1);
    return 0;
}