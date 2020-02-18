#include "conveyor_belt.h"

int queue_size, max_weight, truck_max_weight, number_of_loaders;
int opt_arg = 0;
char *cycles;

void sigint_handler(int signo){
    shmctl(shm_id,IPC_RMID,NULL);
    semctl(sem_id, 0, IPC_RMID);
    printf("\n\nSIGINT\n\n");
    exit(0);
}

void prepare_memory(){
    key_t key = ftok(".",PROJECT_ID);
    shm_id = shmget(key,6*sizeof(int)+queue_size*sizeof(struct product),IPC_CREAT | IPC_EXCL | 0666);//6*sizeof(int)+queue_size*sizeof(struct product) = sizeof(queue)
    sem_id = semget(key, 1024, IPC_CREAT | 0600);
    union semun semun;
    semun.val=0;
    semctl(sem_id, 0, SETVAL, semun);
    queue = shmat(shm_id,NULL,0);
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

    struct sembuf sembuf;
    sembuf.sem_num=0;
    sembuf.sem_op=-1;
    semop(sem_id,&sembuf,1);

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
        sembuf.sem_op=1;
        semop(sem_id,&sembuf,1);
        sembuf.sem_op=-1;
        semop(sem_id,&sembuf,1);
    }
    sigint_handler(SIGINT);
    return 0;
}