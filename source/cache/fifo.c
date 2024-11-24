#include "fifo.h"

#include <stdio.h>

#include "../structures/lookup.h"
#include "../structures/queue.h"

#include "../utils/string.h"

typedef struct fifo {
    lookup_t f_lookup;
    queue_t f_queue;
} fifo_t;

unsigned long priority_insert_fifo() {
    return time(NULL);
}

unsigned long priority_update_fifo(unsigned long prev) {
    return prev;
}

void init_fifo(cache_t * cache) {

    fifo_t * fifo = malloc(sizeof(fifo_t));
    cache->c_data = fifo;

    init_lookup(&fifo->f_lookup, 4096, hash_string, equal_string);
    init_queue(&fifo->f_queue, 4096);
}

void free_fifo(cache_t * cache) {

    fifo_t * fifo = cache->c_data;

    free_lookup(&fifo->f_lookup);
    free_queue(&fifo->f_queue);

    free(cache->c_data);
    cache->c_data = NULL;
}

void insert_fifo(cache_t * cache, cache_entry_t * entry) {

    fifo_t * fifo = cache->c_data;

    insert_lookup(&fifo->f_lookup, entry->ce_source, entry);
    push_queue(&fifo->f_queue, entry);
}

cache_entry_t * remove_fifo(cache_t * cache, char * path) {
    printf("Error: remove item from fifo is not supported!\n");
    exit(EXIT_FAILURE);
}

cache_entry_t * retrieve_fifo(cache_t * cache, char * path) {

    fifo_t * fifo = cache->c_data;

    cache_entry_t * entry = retreive_lookup(&fifo->f_lookup, path);
    if (entry == NULL) {
        return NULL;
    }

    return entry;
}

cache_entry_t * pop_fifo(cache_t * cache) {

    fifo_t * fifo = cache->c_data;

    cache_entry_t * entry = pop_queue(&fifo->f_queue);
    if (entry == NULL) {
        return NULL;
    }

    remove_lookup(&fifo->f_lookup, entry->ce_source);

    return entry;
}