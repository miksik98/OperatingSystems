#include "conveyor_belt.h"

int is_empty(struct queue *queue){
    if(queue->number_of_products == 0) return 1;
    else return 0;
}

int is_full(struct queue *queue){
    if(queue->number_of_products == queue->queue_size) return 1;
    else return 0;
}

struct product first_in_queue(struct queue *queue){
    return queue->conveyor_belt[queue->front];
}

int size(struct queue *queue){
    return queue->number_of_products;
}

int insert(struct product prod, struct queue *queue){
    if(!is_full(queue)) {

        if(queue->end == queue->queue_size-1) queue->end = -1;

        int weight_after = queue->current_weight+prod.val;

        if(weight_after>queue->max_weight) return 0; // couldn't insert

        queue->end++;
        queue->conveyor_belt[queue->end] = prod;
        queue->number_of_products++;
        queue->current_weight = weight_after;
        return 1;
    }
    return 0;
}

struct product take_product(struct queue *queue) {
    struct product product_taken = queue->conveyor_belt[queue->front];
    queue->front++;

    if(queue->front == queue->queue_size) queue->front = 0;

    queue->current_weight -= product_taken.val;
    queue->number_of_products--;
    return product_taken;
}