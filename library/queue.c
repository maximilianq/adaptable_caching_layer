#define _GNU_SOURCE

#include "queue.h"

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define QUEUE_SIZE 16384

void init_queue(queue_t * queue) {
    queue->data = malloc(QUEUE_SIZE * sizeof(char *));
    queue->size = 0;
    queue->front = 0;
    queue->back = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    sem_init(&queue->empty, 0, 0);
    sem_init(&queue->full, 0, QUEUE_SIZE);
}

void free_queue(queue_t * queue) {
    free(queue->data);
    pthread_mutex_destroy(&queue->mutex);
    sem_destroy(&queue->empty);
    sem_destroy(&queue->full);
}

void enqueue(queue_t * queue, char * data) {

    sem_wait(&queue->full);
    pthread_mutex_lock(&queue->mutex);

    queue->data[queue->back] = data;
    queue->back = (queue->back + 1) % QUEUE_SIZE;
    queue->size++;

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->empty);
}

char * dequeue(queue_t * queue) {

    sem_wait(&queue->empty);
    pthread_mutex_lock(&queue->mutex);

    char * item = queue->data[queue->front];
    queue->front = (queue->front + 1) % QUEUE_SIZE;
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->full);

    return item;
}