#include "heap.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

int upheap(heap_t * heap, int index) {

    while (index != 0 && heap->h_key[(index - 1) / 2] > heap->h_key[index]) {

        int parent = (index - 1) / 2;

        int temp_key = heap->h_key[index];
        heap->h_key[index] = heap->h_key[parent];
        heap->h_key[parent] = temp_key;

        void *temp_data = heap->h_data[index];
        heap->h_data[index] = heap->h_data[parent];
        heap->h_data[parent] = temp_data;

        index = parent;
    }

    return index;
}

int downheap(heap_t * heap, int index) {

    int l = (2 * index) + 1;
    int r = (2 * index) + 2;
    int smalles = index;

    if (l < heap->h_size && heap->h_key[l] < heap->h_key[smalles]) {
        smalles = l;
    }

    if (r < heap->h_size && heap->h_key[r] < heap->h_key[smalles]) {
        smalles = r;
    }

    if (smalles != index) {

        int temp_key = heap->h_key[index];
        heap->h_key[index] = heap->h_key[smalles];
        heap->h_key[smalles] = temp_key;

        void * temp_data = heap->h_data[index];
        heap->h_data[index] = heap->h_data[smalles];
        heap->h_data[smalles] = temp_data;

        index = downheap(heap, smalles);
    }

    return index;
}

int insert(heap_t * heap, unsigned long key, void * data) {

    sem_wait(&heap->h_full);
    pthread_mutex_lock(&heap->h_mutex_atomic);

    int index = heap->h_size;

    heap->h_key[index] = key;
    heap->h_data[index] = data;

    upheap(heap, index);

    heap->h_size++;

    pthread_mutex_unlock(&heap->h_mutex_atomic);
    sem_post(&heap->h_empty);

    return index;
}

void * extract(heap_t * heap) {

    sem_wait(&heap->h_empty);
    pthread_mutex_lock(&heap->h_mutex_atomic);

    if (heap->h_size == 1) {

        heap->h_size--;

        pthread_mutex_unlock(&heap->h_mutex_atomic);
        sem_post(&heap->h_full);

        return heap->h_data[0];
    }

    void * result = heap->h_data[0];

    heap->h_key[0] = heap->h_key[heap->h_size - 1];
    heap->h_data[0] = heap->h_data[heap->h_size - 1];

    heap->h_size--;

    downheap(heap, 0);

    pthread_mutex_unlock(&heap->h_mutex_atomic);
    sem_post(&heap->h_full);

    return result;
}

int update(heap_t * heap, int index, unsigned long key) {

    pthread_mutex_lock(&heap->h_mutex_atomic);

    if (key < heap->h_key[index]) {
        heap->h_key[index] = key;
        index = upheap(heap, index);
    } else {
        heap->h_key[index] = key;
        index = downheap(heap, index);
    }

    pthread_mutex_unlock(&heap->h_mutex_atomic);

    return index;
}

int find(heap_t * heap, const void * search) {

    pthread_mutex_lock(&heap->h_mutex_atomic);

    int index = -1;

    for (int i = 0; i < heap->h_size; i++) {
        if (heap->h_predicate(heap->h_data[i], search)) {
            index = i;
            break;
        }
    }

    pthread_mutex_unlock(&heap->h_mutex_atomic);

    return index;
}

void init_heap(heap_t * heap, int capacity, transform_t transform, predicate_t predicate) {
    heap->h_key = malloc(sizeof(int) * capacity);
    heap->h_data = malloc(sizeof(void *) * capacity);
    heap->h_size = 0;
    pthread_mutex_init(&heap->h_mutex_atomic, NULL);
    pthread_mutex_init(&heap->h_mutex_operation, NULL);
    sem_init(&heap->h_empty, 0, 0);
    sem_init(&heap->h_full, 0, capacity);
    heap->h_transform = transform;
    heap->h_predicate = predicate;
}

void free_heap(heap_t * heap) {
    free(heap->h_data);
    pthread_mutex_destroy(&heap->h_mutex_atomic);
    pthread_mutex_destroy(&heap->h_mutex_operation);
    sem_destroy(&heap->h_empty);
    sem_destroy(&heap->h_full);
}

int insert_heap(heap_t * heap, unsigned long key, const void * search, void * data, void * result, size_t size) {

    int index;

    pthread_mutex_lock(&heap->h_mutex_operation);

    if ((index = find(heap, search)) != -1) {
        memcpy(result, heap->h_data[index], size);
        pthread_mutex_unlock(&heap->h_mutex_operation);
        return 1;
    }

    if ((index = insert(heap, key, data)) == -1) {
        pthread_mutex_unlock(&heap->h_mutex_operation);
        return -1;
    }

    memcpy(result, heap->h_data[index], size);

    pthread_mutex_unlock(&heap->h_mutex_operation);

    return 0;
}

int extract_heap(heap_t * heap, void * result, size_t size) {

    void * entry;

    pthread_mutex_lock(&heap->h_mutex_operation);

    if ((entry = extract(heap)) == NULL) {
        pthread_mutex_unlock(&heap->h_mutex_operation);
        return -1;
    }

    memcpy(result, entry, size);

    free(entry);

    pthread_mutex_unlock(&heap->h_mutex_operation);

    return 0;
}

int update_heap(heap_t * heap, const void * search, void * result, size_t size) {

    int index;

    pthread_mutex_lock(&heap->h_mutex_operation);

    if ((index = find(heap, search)) == -1) {
        pthread_mutex_unlock(&heap->h_mutex_operation);
        return -1;
    }

    index = update(heap, index, heap->h_transform(heap->h_key[index]));

    memcpy(result, heap->h_data[index], size);

    pthread_mutex_unlock(&heap->h_mutex_operation);

    return 0;
}