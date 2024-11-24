#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <limits.h>

#include "structures/extent.h"

// forward declaration of prefetch_t for utilization in prefetch_t due to circular dependency
typedef struct cache cache_t;
typedef struct mapping mapping_t;

typedef struct cache_entry {
    char ce_source[PATH_MAX];
    char ce_cache[PATH_MAX];
    size_t ce_size;
    time_t ce_mtime;
    extent_t ce_extents;
    pthread_mutex_t ce_mutex;
} cache_entry_t;


typedef void (* init_cache_t)(cache_t * cache);
typedef void (* free_cache_t)(cache_t * cache);
typedef void (* insert_cache_t)(cache_t * cache, cache_entry_t * entry);
typedef void (* inserted_cache_t)(cache_t * cache, cache_entry_t * entry);
typedef cache_entry_t * (* remove_cache_t)(cache_t * cache, char * path);
typedef void (* removed_cache_t)(cache_t * cache, cache_entry_t * entry);
typedef cache_entry_t * (* retrieve_cache_t)(cache_t * cache, char * path);
typedef cache_entry_t * (* pop_cache_t)(cache_t * cache);

typedef struct cache_policy {
    init_cache_t cp_init;
    free_cache_t cp_free;
    insert_cache_t cp_insert;
    remove_cache_t cp_remove;
    retrieve_cache_t cp_retrieve;
    pop_cache_t cp_pop;
} cache_policy_t;

typedef struct cache {
    size_t c_capacity;
    size_t c_size;
    mapping_t * c_mapping;
    cache_policy_t c_policy;
    inserted_cache_t c_inserted;
    removed_cache_t c_removed;
    void * c_data;
    pthread_mutex_t c_mutex;
} cache_t;

void init_cache(cache_t * cache, mapping_t * mapping, ssize_t capacity, cache_policy_t policy, inserted_cache_t inserted, removed_cache_t removed);

void free_cache(cache_t * cache);

void cache_replace(cache_t * cache, cache_policy_t cache_policy);

int insert_cache(cache_t * cache, char * path);

cache_entry_t * retrieve_cache(cache_t * cache, char * path);

int remove_cache(cache_t * cache, char * path);

int pop_cache(cache_t * cache);

int hash_cache_entry(void * data);

int equal_cache_entry(void * first, void * second);

#endif
