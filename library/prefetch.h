#ifndef PREFETCH_H
#define PREFETCH_H

#include "cache.h"

#include "structures/queue.h"

// forward declaration of prefetch_t for utilization in prefetch_t due to circular dependency
typedef struct prefetch prefetch_t;

// functions to be implemented by custom prefetching logic
typedef void (* init_t) (prefetch_t * prefetch);
typedef void (* free_t) (prefetch_t * prefetch);
typedef void (* process_t) (prefetch_t * prefetch, char * path);

typedef struct prefetch {

    queue_t         p_history;
    queue_t         p_high;
    queue_t         p_low;

    init_t          p_init;
    free_t          p_free;
    process_t       p_process;

    pthread_t       p_thread;
    int             p_status;
    void *          p_data;

} prefetch_t;

typedef struct arguments {
    prefetch_t *    a_prefetch;
    cache_t *       a_cache;
} arguments_t;

void prefetch_init(prefetch_t * prefetch, init_t init, process_t process, free_t free);

void prefetch_free(prefetch_t * prefetch);

void prefetch_replace(prefetch_t * prefetch, cache_t * cache, init_t init, process_t process, free_t free);

void prefetch_predict(prefetch_t * prefetch, char * path);

void * prefetch_handler(void * data);

#endif