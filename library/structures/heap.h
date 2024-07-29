#ifndef heap_H
#define heap_H

#include <semaphore.h>

typedef unsigned long (* increment_t)(unsigned long key);
typedef unsigned long (* transform_t)(unsigned long key);
typedef int (* predicate_t)(const void * data, const void * search);

typedef struct {
    unsigned long * h_key;
    void ** h_data;
    int h_size;
    pthread_mutex_t h_mutex_atomic, h_mutex_operation;
    sem_t h_full, h_empty;
    transform_t h_transform;
    predicate_t h_predicate;
} heap_t;

void init_heap(heap_t * heap, int capacity, transform_t transform, predicate_t predicate);

void free_heap(heap_t * heap);

int insert_heap(heap_t * heap, unsigned long key, const void * search, void * data, void * result, size_t size);

int extract_heap(heap_t * heap, void * result, size_t size);

int update_heap(heap_t * heap, const void * search, void * result, size_t size);

#endif
