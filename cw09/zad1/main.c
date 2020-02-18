#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int is_a_number(char * string){
    for (int i = 0; i < strlen(string); i++){
        if (((int)string[i]>57)||((int)string[i]<48)) return 0; // not digit
    }
    return 1;
}


pthread_t *passengers_tids;
pthread_t *carts_tids;

int *carts;
int *passengers;

int passengers_number;
int carts_number;
int cart_capacity;
int runs_number;
int runs_left;
int current_id = 0;
int *carts_current_capacity;
int current_id_finish = 0;

pthread_mutex_t load_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t load_cart_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t load_passenger_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t load_passenger_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t unload_passenger_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t unload_passenger_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t unload_cart_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t unload_cart_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t full_cart_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_cart_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t empty_cart_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty_cart_cond = PTHREAD_COND_INITIALIZER;

int loading = 0;
int unloading = 0;

void *passenger(void *args) {
    int id = *((int*)args);
    int cart;

    while(1) {
        pthread_mutex_lock(&load_passenger_mutex);
        while (!loading) {
            pthread_cond_wait(&load_passenger_cond, &load_passenger_mutex);
        }

        cart = current_id;
        carts_current_capacity[cart]++;
        printf("  Passenger %d enters cart %d - actual capacity: %d/%d\n", id, cart, carts_current_capacity[cart], cart_capacity);

        if (carts_current_capacity[cart] < cart_capacity) {
            pthread_mutex_unlock(&load_passenger_mutex);
            pthread_cond_broadcast(&load_passenger_cond);
        }
        else {
            loading = 0;
            printf("  Passenger %d pressed the START!\n", id);
            pthread_mutex_unlock(&load_passenger_mutex);
            pthread_cond_broadcast(&full_cart_cond);
        }

        pthread_mutex_lock(&unload_passenger_mutex);
        while (!unloading || cart != current_id_finish) {
            pthread_cond_wait(&unload_passenger_cond, &unload_passenger_mutex);
        }

        carts_current_capacity[cart]--;
        printf("   Passenger %d leaves cart %d - actual capacity: %d/%d\n", id, cart, cart_capacity-carts_current_capacity[cart], cart_capacity);

        if (carts_current_capacity[cart] > 0) {
            cart = -1;
            pthread_mutex_unlock(&unload_passenger_mutex);
            pthread_cond_broadcast(&unload_passenger_cond);
        }
        else {
            cart = -1;
            unloading = 0;
            pthread_mutex_unlock(&unload_passenger_mutex);
            pthread_cond_broadcast(&empty_cart_cond);
        }
    }
}

void *cart(void *args) {
    int id = *((int*)args);

    while(1) {
        pthread_mutex_lock(&load_mutex);
        if(runs_left == 0) break;

        while (id != current_id) {
            pthread_cond_wait(&load_cart_cond, &load_mutex);
        }

        printf("Cart %d is ready to be loaded\n", id);
        loading = 1;
        pthread_cond_broadcast(&load_passenger_cond);

        while (carts_current_capacity[id] < cart_capacity){
            pthread_cond_wait(&full_cart_cond, &full_cart_mutex);
        }

        printf("Cart %d makes its %d/%d run\n", id, runs_number-runs_left + 1, runs_number);

        if (current_id == carts_number-1) runs_left--;

        current_id = (current_id+1)%carts_number;
        pthread_mutex_unlock(&load_mutex);
        pthread_cond_broadcast(&load_cart_cond);
        usleep(1000000);

        pthread_mutex_lock(&unload_cart_mutex);
        while (id != current_id_finish) {
            pthread_cond_wait(&unload_cart_cond, &unload_cart_mutex);
        }

        printf("Cart %d finished a run. Now it's unloading passengers:\n", id);
        unloading = 1;

        pthread_cond_broadcast(&unload_passenger_cond);
        while (carts_current_capacity[id] > 0){
            pthread_cond_wait(&empty_cart_cond, &empty_cart_mutex);
        }

        current_id_finish = (current_id_finish+1)%carts_number;
        printf("Cart %d is empty and waiting\n", id);
        pthread_mutex_unlock(&unload_cart_mutex);
        pthread_cond_broadcast(&unload_cart_cond);
    }
    printf("Cart %d just finished all its runs\n", id);
    pthread_mutex_unlock(&load_mutex);
    pthread_exit(NULL);
}

void clean_up(){
    free(carts_tids);
    free(passengers_tids);
    free(carts);
    free(passengers);
    free(carts_current_capacity);
}

int main(int argc, char * argv[]) {
    if (argc != 5) {
        printf("Wrong number of args!\n");
        printf("Correct input: <number of passangers> <number of carts> <capacity> <runs>\n");
        exit(1);
    }

    for (int i = 1; i < 5; i++){
        if (!is_a_number(argv[i])){
            printf("All args should be nonnegative numbers\n");
            exit(1);
        }
    }

    passengers_number = atoi(argv[1]);
    carts_number = atoi(argv[2]);
    cart_capacity = atoi(argv[3]);
    runs_number = atoi(argv[4]);

    carts_tids = calloc(carts_number, sizeof(pthread_t));
    passengers_tids = calloc(passengers_number, sizeof(pthread_t));
    runs_left = runs_number;
    carts = calloc(carts_number, sizeof(int));
    passengers = calloc(passengers_number, sizeof(int));
    carts_current_capacity = calloc(carts_number, sizeof(int));

    atexit(clean_up);

    for (int i = 0; i < carts_number; i++){
        carts[i] = i;
        carts_current_capacity[i] = 0;
        pthread_create(&carts_tids[i], NULL, cart, (void*)&carts[i]);
    }

    for (int i = 0; i < passengers_number; i++){
        passengers[i] = i;
        pthread_create(&passengers_tids[i], NULL, passenger, (void*)&passengers[i]);
    }

    for (int i = 0; i < carts_number; i++){
        pthread_join(carts_tids[i], NULL);
    }

    for (int i = 0; i < passengers_number; i++){
        pthread_cancel(passengers_tids[i]);
    }

    return 0;
}