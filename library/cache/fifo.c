#include "fifo.h"

#include "../structures/lookup.h"
#include "../structures/priority.h"

#include "../utils/string.h"

typedef struct fifo {
    lookup_t f_lookup;
    priority_t f_priority;
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
    init_priority(&fifo->f_priority, 4096, hash_cache_entry, equal_cache_entry, PRIO_MIN, priority_insert_fifo, priority_update_fifo);
}

void free_fifo(cache_t * cache) {

    fifo_t * fifo = cache->c_data;

    free_lookup(&fifo->f_lookup);
    free_priority(&fifo->f_priority);

    free(cache->c_data);
    cache->c_data = NULL;
}

void insert_fifo(cache_t * cache, cache_entry_t * entry) {

    fifo_t * fifo = cache->c_data;

    insert_lookup(&fifo->f_lookup, entry->ce_source, entry);
    insert_priority(&fifo->f_priority, time(NULL), entry);
}

cache_entry_t * remove_fifo(cache_t * cache, char * path) {

    fifo_t * fifo = cache->c_data;

    cache_entry_t * entry = remove_lookup(&fifo->f_lookup, path);
    if (entry == NULL) {
        return NULL;
    }

    remove_priority(&fifo->f_priority, entry);

    return entry;
}

cache_entry_t * retrieve_fifo(cache_t * cache, char * path) {

    fifo_t * fifo = cache->c_data;

    cache_entry_t * entry = retreive_lookup(&fifo->f_lookup, path);
    if (entry == NULL) {
        return NULL;
    }

    update_priority(&fifo->f_priority, time(NULL), entry);

    return entry;
}

cache_entry_t * pop_fifo(cache_t * cache) {

    fifo_t * fifo = cache->c_data;

    cache_entry_t * entry = pop_priority(&fifo->f_priority);
    if (entry == NULL) {
        return NULL;
    }

    remove_lookup(&fifo->f_lookup, entry->ce_source);

    return entry;
}