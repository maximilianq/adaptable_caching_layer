#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <limits.h>

#include "structures/queue.h"
#include "structures/heap.h"

typedef struct cache_mapping {
    int cm_file;
    char cm_source[PATH_MAX];
    char cm_cache[PATH_MAX];
    int cm_flags;
    mode_t cm_mode;
    size_t cm_lower;
    size_t cm_upper;
} cache_mapping_t;

typedef struct cache_entry {
    char ce_source[PATH_MAX];
    char ce_cache[PATH_MAX];
    pthread_mutex_t ce_mutex;
    int ce_status;
    size_t ce_size;
    time_t ce_mtime;
} cache_entry_t;

typedef struct cache {
    size_t c_capacity;
    size_t c_size;
    queue_t c_prefetch_entries;
    heap_t c_cache_entries;
    pthread_mutex_t c_mutex_cache;
    cache_mapping_t * c_mapping[1024];
    pthread_mutex_t c_mutex_mapping[1024];
} cache_t;

#endif