#define _GNU_SOURCE

#include "queue.h"

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <asm-generic/errno-base.h>

void init_queue(queue_t * queue, int capacity) {
    queue->data = malloc(capacity * sizeof(void *));
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->back = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    sem_init(&queue->empty, 0, 0);
    sem_init(&queue->full, 0, capacity);
}

void free_queue(queue_t * queue) {
    free(queue->data);
    pthread_mutex_destroy(&queue->mutex);
    sem_destroy(&queue->empty);
    sem_destroy(&queue->full);
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

    if (sem_trywait(&queue->empty) == -1) {

        if (errno != EAGAIN) {
            perror("ERROR: could not wait for semaphore!");
            exit(EXIT_FAILURE);
        }

        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);

    void * item = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);

    return item;
}