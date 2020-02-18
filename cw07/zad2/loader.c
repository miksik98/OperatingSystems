#include "conveyor_belt.h"

int weight_of_product;

void prepare_memory(){
    shm_id = shm_open(SHARED_MEM, O_RDWR, 0666);
    if(shm_id==-1)
    {
        printf("Error with getting shared memory\n");
        exit(1);
    }
    queue = mmap(NULL, sizeof(struct product), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    size_t len = 6* sizeof(int) + (*queue).queue_size*sizeof(struct product);
    queue= mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
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
    sem_id = sem_open(SEMAPHORE, 0666);
    time_t rawtime;
    int packages_loaded = 0;
    sem_post(sem_id);
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
        sem_post(sem_id);
        sem_wait(sem_id);
    }
    printf("Loader with id: %d finished. Loaded %d packages\n", getpid(), cycles);
    sem_post(sem_id);
    return 0;
}