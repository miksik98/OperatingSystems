#include "conveyor_belt.h"

int queue_size, max_weight, truck_max_weight, number_of_loaders;
int opt_arg = 0;
char *cycles;

void sigint_handler(int signo){
    sem_close(sem_id);
    sem_unlink(SEMAPHORE);
    munmap(queue, 6*sizeof(int)+queue_size* sizeof(struct product));
    shm_unlink(SHARED_MEM);
    printf("\n\nSIGINT\n\n");
    exit(0);
}

void prepare_memory(){
    shm_id = shm_open(SHARED_MEM, O_RDWR | O_CREAT, 0666);
    ftruncate(shm_id,6*sizeof(int)+queue_size* sizeof(struct product));
    queue = mmap(NULL, 6*sizeof(int)+queue_size* sizeof(struct product), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
}

void exec_loaders(){
    int max = max_weight, random_weight;
    char *weight = malloc(sizeof(char*));
    for(int i = 0; i<number_of_loaders; i++){
        random_weight = rand()%max+1;
        sprintf(weight, "%d", random_weight);
        if(fork()==0){
            if(opt_arg)
                execl("./loader","./loader",weight,cycles,NULL);
            else
                execl("./loader","./loader",weight,NULL);
        }
    }
}
int main(int argc, char **argv)
{
    if(argc!=5 && argc!=6){
        printf("Bad number of arguments. See Readme.txt.\n");
        exit(1);
    }
    srand(time(0));
    truck_max_weight = atoi(argv[1]);
    queue_size = atoi(argv[2]);
    max_weight = atoi(argv[3]);
    number_of_loaders = atoi(argv[4]);
    if(argc == 6){
        cycles = argv[5];
        opt_arg = 1;
    }

    signal(SIGINT, sigint_handler);
    prepare_memory();

    queue->front = 0;
    queue->end = -1;
    queue->number_of_products = 0;
    queue->queue_size = queue_size;
    queue->max_weight = max_weight;
    queue->current_weight = 0;

    int current_truck_weight = 0;
    struct product product;

    time_t rawtime;
    time (&rawtime);

    printf("Time: %ld ",rawtime);
    printf("New truck is arriving!\n");

    exec_loaders();

    sem_id = sem_open(SEMAPHORE, O_CREAT | O_EXCL, 0666, 1);

    sem_wait(sem_id);
    while(1){
        if(!is_empty(queue)){
            if(first_in_queue(queue).val+current_truck_weight > truck_max_weight){
                time (&rawtime);
                printf("Time: %ld ",rawtime);
                queue->current_weight = 0;
                current_truck_weight = 0;
                printf("No more space: new truck arriving!\n");
            }
            else{
                product = take_product(queue);
                current_truck_weight += product.val;
                time(&rawtime);
                printf("Time: %ld ",rawtime);
                printf("Pack is being loaded on truck | loader_id: %d  | weight of product: %d | free: %d | taken: %d | time_diff: %ld\n", product.loader_id, product.val,truck_max_weight-current_truck_weight,current_truck_weight, rawtime-product.date_add_to_queue);
            }
        }
        else{
            time (&rawtime);
            printf("Time: %ld ",rawtime);
            printf("Empty belt, waiting for products!\n");
        }

        sem_post(sem_id);
        sem_wait(sem_id);
    }
    sigint_handler(SIGINT);
    return 0;
}