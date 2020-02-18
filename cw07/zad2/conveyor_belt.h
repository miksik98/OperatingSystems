#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>

#define PROJECT_ID 'a'
#define MAX_QUEUE_LENGTH 100
#define SHARED_MEM "/shared"
#define SEMAPHORE "/semaphore"

struct queue *queue;
int shm_id;
sem_t *sem_id;

struct product{
    int val;
    long date_add_to_queue;
    pid_t loader_id;
};

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

struct queue{
    struct product conveyor_belt[MAX_QUEUE_LENGTH];
    int front;
    int end;
    int number_of_products;
    int queue_size;
    int max_weight;
    int current_weight;
};

int is_empty(struct queue *queue);

int is_full(struct queue *queue);

struct product first_in_queue(struct queue *queue);

int size(struct queue *queue);

int insert(struct product prod, struct queue *queue);

struct product take_product(struct queue *queue);
