#include "heap.h"

void init_heap(heap_t * heap, int capacity, compare_t compare, listen_t listen, void * listen_args) {
    init_internal_heap(&heap->h_internal, capacity, compare, listen, listen_args);
    pthread_mutex_init(&heap->h_mutex, NULL);
    sem_init(&heap->h_empty, 0, 0);
    sem_init(&heap->h_full, 0, capacity);
}

void free_heap(heap_t * heap) {
    free_internal_heap(&heap->h_internal);
    pthread_mutex_destroy(&heap->h_mutex);
    sem_destroy(&heap->h_empty);
    sem_destroy(&heap->h_full);
}

int insert_heap(heap_t * heap, unsigned long score, void * data) {

    // wait for heap to be not full and lock heap for modification
    sem_wait(&heap->h_full);
    pthread_mutex_lock(&heap->h_mutex);

    // delegate to internal heap
    int result = insert_internal_heap(&heap->h_internal, score, data);

    // unlock heap and signal for heap not empty
    pthread_mutex_unlock(&heap->h_mutex);
    sem_post(&heap->h_empty);

    // return current index of entry
    return result;
}

void * remove_heap(heap_t * heap, int index) {

    // wait for heap to be not empty and lock heap for modification
    sem_wait(&heap->h_empty);
    pthread_mutex_lock(&heap->h_mutex);

    // delegate to internal heap
    void * result = remove_internal_heap(&heap->h_internal, index);

    // unlock heap and signal for heap not full
    pthread_mutex_unlock(&heap->h_mutex);
    sem_post(&heap->h_full);

    // return removed entry
    return result;
}

int update_heap(heap_t * heap, int index, unsigned long score) {

    pthread_mutex_lock(&heap->h_mutex);

    // delegate to internal heap
    int result = update_internal_heap(&heap->h_internal, index, score);

    pthread_mutex_unlock(&heap->h_mutex);

    return result;
}