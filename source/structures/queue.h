#ifndef QUEUEU_H
#define QUEUEU_H

#include "internal/queue.h"

typedef struct {
    internal_queue_t q_internal;
    pthread_mutex_t q_mutex;
} queue_t;

void init_queue(queue_t * queue, int capacity);

void free_queue(queue_t * queue);

void push_queue(queue_t * queue, void * data);

void * pop_queue(queue_t * queue);

#endif
