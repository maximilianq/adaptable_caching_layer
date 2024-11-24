#ifndef PRIORITY_H
#define PRIORITY_H

#include <pthread.h>

#include "internal/priority.h"

typedef struct priority_queue {
    internal_priority_t p_internal;
    pthread_mutex_t p_mutex;
} priority_t;

void init_priority(priority_t * priority, int capacity, hash_t hash, equal_t equal, int kind, insert_t insert, update_t update);

void free_priority(priority_t * priority);

void insert_priority(priority_t * priority, unsigned long score, void * data);

void update_priority(priority_t * priority, unsigned long score, void * data);

int remove_priority(priority_t * priority, void * data);

void * pop_priority(priority_t * priority);

#endif