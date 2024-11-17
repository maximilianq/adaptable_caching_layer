#ifndef INTERNAL_QUEUEU_H
#define INTERNAL_QUEUEU_H

#include <semaphore.h>

typedef struct {
    void ** data;
    int capacity, size, front, back;
    sem_t full, empty;
} internal_queue_t;

void init_internal_queue(internal_queue_t * queue, int capacity);

void free_internal_queue(internal_queue_t * queue);

void push_internal_queue(internal_queue_t * queue, void * data);

void * pop_internal_queue(internal_queue_t * queue);

#endif
