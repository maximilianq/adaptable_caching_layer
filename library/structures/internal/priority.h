#ifndef INTERNAL_PRIORITY_H
#define INTERNAL_PRIORITY_H

#include "lookup.h"
#include "heap.h"

#define PRIO_MIN 0
#define PRIO_MAX 1

typedef struct internal_priority {
    internal_heap_t p_heap;
    internal_lookup_t p_lookup;
    pthread_mutex_t p_mutex;
} internal_priority_t;

void init_internal_priority(internal_priority_t * priority, int capacity, hash_t hash, equal_t equal, int kind);

void free_internal_priority(internal_priority_t * priority);

void insert_internal_priority(internal_priority_t * priority, unsigned long score, void * data);

void update_internal_priority(internal_priority_t * priority, unsigned long score, void * data);

void * remove_internal_priority(internal_priority_t * priority);

#endif