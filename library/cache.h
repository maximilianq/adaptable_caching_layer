#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <limits.h>
#include <stdbool.h>

#include "queue.h"

typedef struct cache_mapping {
    int cm_file;
    char cm_path[PATH_MAX];
    mode_t cm_mode;
    size_t cm_lower;
    size_t cm_upper;
} cache_mapping_t;

typedef struct cache_entry {

    char ce_source[PATH_MAX];
    char ce_cache[PATH_MAX];

    bool ce_ready;

    size_t ce_size;

    time_t ce_recency;

    struct cache_entry * ce_previos;
    struct cache_entry * ce_next;

} cache_entry_t;

typedef struct cache {

    cache_entry_t * c_head;
    cache_entry_t * c_tail;

    int c_entries;

    size_t c_capacity;
    size_t c_size;

    queue_t c_lookahead;

    cache_mapping_t * c_mapping[1024];

    pthread_mutex_t c_mutex_cache;
    pthread_mutex_t c_mutex_mapping[1024];
} cache_t;

void cache_init(cache_t * cache);

void cache_free(cache_t * cache);

void cache_update(cache_t * cache, const char * source_path);

char * cache_query(cache_t * cache, const char * source_path);

#endif
