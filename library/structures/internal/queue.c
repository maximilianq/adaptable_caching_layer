#define _GNU_SOURCE

#include "queue.h"

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void init_queue(queue_t * queue, int capacity) {
    queue->data = malloc(capacity * sizeof(void *));
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->back = 0;
}

void free_queue(queue_t * queue) {
    free(queue->data);
}

void enqueue(queue_t * queue, void * data) {

    pthread_mutex_lock(&queue->mutex);

    if (queue->size < queue->capacity) {
        queue->data[queue->back] = data;
        queue->back = (queue->back + 1) % queue->capacity;
        queue->size++;
        sem_post(&queue->empty);
    }

    pthread_mutex_unlock(&queue->mutex);
}

void * dequeue(queue_t * queue) {

    sem_wait(&queue->empty);

    pthread_mutex_lock(&queue->mutex);

    void * item = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);

    return item;
}