#define _GNU_SOURCE

#include "queue.h"

#include <pthread.h>

void init_queue(queue_t * queue, int capacity) {
    init_internal_queue(&queue->q_internal, capacity);
    pthread_mutex_init(&queue->q_mutex, NULL);
}

void free_queue(queue_t * queue) {
    free_internal_queue(&queue->q_internal);
    pthread_mutex_destroy(&queue->q_mutex);
}

void push_queue(queue_t * queue, void * data) {
    pthread_mutex_lock(&queue->q_mutex);
    push_internal_queue(&queue->q_internal, data);
    pthread_mutex_unlock(&queue->q_mutex);
}

void * pop_queue(queue_t * queue) {
    pthread_mutex_lock(&queue->q_mutex);
    void * data = pop_internal_queue(&queue->q_internal);
    pthread_mutex_unlock(&queue->q_mutex);
    return data;
}