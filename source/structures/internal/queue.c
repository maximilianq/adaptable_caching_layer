#define _GNU_SOURCE

#include "queue.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

void init_internal_queue(internal_queue_t * queue, int capacity) {
    queue->data = malloc(capacity * sizeof(void *));
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->back = 0;
    sem_init(&queue->empty, 0, 0);
    sem_init(&queue->full, 0, capacity);
}

void free_internal_queue(internal_queue_t * queue) {
    free(queue->data);
    sem_destroy(&queue->empty);
    sem_destroy(&queue->full);
}

void push_internal_queue(internal_queue_t * queue, void * data) {

    if (sem_trywait(&queue->full) == -1) {

        if (errno != EAGAIN) {
            perror("ERROR: could not wait for semaphore!");
            exit(EXIT_FAILURE);
        }

        return;
    }

    queue->data[queue->back] = data;
    queue->back = (queue->back + 1) % queue->capacity;
    queue->size++;

    sem_post(&queue->empty);
}

void * pop_internal_queue(internal_queue_t * queue) {

    if (sem_trywait(&queue->empty) == -1) {

        if (errno != EAGAIN) {
            perror("ERROR: could not wait for semaphore!");
            exit(EXIT_FAILURE);
        }

        return NULL;
    }

    void * item = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    sem_post(&queue->full);

    return item;
}