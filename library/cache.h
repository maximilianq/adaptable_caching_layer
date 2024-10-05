#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <limits.h>

#include "structures/priority.h"
#include "structures/lookup.h"
#include "structures/extent.h"

typedef struct cache_entry {
    char ce_source[PATH_MAX];
    char ce_cache[PATH_MAX];
    size_t ce_size;
    time_t ce_mtime;
    extent_t ce_extents;
    pthread_mutex_t ce_mutex;
} cache_entry_t;

typedef void (* inserted_t)(cache_entry_t * entry, void * args);

typedef void (* removed_t)(cache_entry_t * entry, void * args);

typedef struct cache {
    size_t c_capacity;
    size_t c_size;

    lookup_t c_lookup;
    priority_t c_priority;

    inserted_t c_inserted;
    void * c_inserted_args;

    removed_t c_removed;
    void * c_removed_args;

    pthread_mutex_t c_mutex;
} cache_t;

void init_cache(cache_t * cache, ssize_t capacity, inserted_t inserted, void * inserted_args, removed_t removed, void * removed_args);

void free_cache(cache_t * cache);

int insert_cache(cache_t * cache, char * path);

cache_entry_t * retrieve_cache(cache_t * cache, char * path);

int remove_cache(cache_t * cache, char * path);

int pop_cache(cache_t * cache);

#endif