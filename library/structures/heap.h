#ifndef HEAP_H
#define HEAP_H

#include <pthread.h>
#include <semaphore.h>

#include "internal/heap.h"

struct heap {
    internal_heap_t h_internal;
    pthread_mutex_t h_mutex;
    sem_t h_full, h_empty;
};

typedef struct heap heap_t;

void init_heap(heap_t * heap, int capacity, compare_t compare, listen_t listen, void * listen_data);

void free_heap(heap_t * heap);

int insert_heap(heap_t * heap, unsigned long score, void * data);

void * remove_heap(heap_t * heap, int index);

int update_heap(heap_t * heap, int index, unsigned long score);

void print_heap(heap_t * heap);

#endif
