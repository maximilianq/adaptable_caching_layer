#ifndef PREFETCH_H
#define PREFETCH_H

#include "cache.h"

#include "structures/queue.h"

#define CM_READ 1
#define CM_WRITE 2

// forward declaration of prefetch_t for utilization in prefetch_t due to circular dependency
typedef struct prefetch prefetch_t;

typedef struct cache_miss {
    char cm_path[PATH_MAX];
    time_t cm_time;
    int cm_operation;
} cache_miss_t;

// functions to be implemented by custom prefetching logic
typedef void (* init_prefetch_t) (prefetch_t * prefetch);
typedef void (* free_prefetch_t) (prefetch_t * prefetch);
typedef void (* process_prefetch_t) (prefetch_t * prefetch, cache_miss_t cache_miss);

typedef struct prefetch_strategy {
    init_prefetch_t ps_init;
    free_prefetch_t ps_free;
    process_prefetch_t ps_process;
} prefetch_strategy_t;

typedef struct prefetch {
    queue_t p_history;
    queue_t p_high;
    queue_t p_low;
    prefetch_strategy_t p_strategy;
    void * p_data;
    pthread_t p_thread;
    int p_status;
} prefetch_t;

typedef struct arguments {
    prefetch_t *    a_prefetch;
    cache_t *       a_cache;
} arguments_t;

void prefetch_init(prefetch_t * prefetch, cache_t * cache, prefetch_strategy_t strategy);

void prefetch_free(prefetch_t * prefetch);

void prefetch_replace(prefetch_t * prefetch, cache_t * cache, prefetch_strategy_t strategy);

void prefetch_predict(prefetch_t * prefetch, char * path);

void * prefetch_handler(void * data);

#endif