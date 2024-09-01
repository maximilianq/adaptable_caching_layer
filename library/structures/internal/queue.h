#ifndef QUEUEU_H
#define QUEUEU_H

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    void ** data;
    int capacity, size, front, back;
    pthread_mutex_t mutex;
    sem_t full, empty;
} queue_t;

void init_queue(queue_t * queue, int capacity);

void free_queue(queue_t * queue);

void enqueue(queue_t * queue, void * data);

void * dequeue(queue_t * queue);

#endif
