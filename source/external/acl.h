#ifndef ACL_H
#define ACL_H

#define ACL_WILLNEED 1
#define ACL_DONTNEED 2

typedef struct prefetch prefetch_t;
typedef struct prefetch cache_t;
typedef struct prefetch cache_entry_t;

// functions to be implemented by custom prefetching logic
typedef void (* init_prefetch_t) (prefetch_t * memory);
typedef void (* free_prefetch_t) (prefetch_t * memory);
typedef void (* process_prefetch_t) (prefetch_t * memory, char * path);

typedef struct prefetch_strategy {
    init_prefetch_t ps_init;
    free_prefetch_t ps_free;
    process_prefetch_t ps_process;
} prefetch_strategy_t;

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

int acl_advise(const char * path, int flags);

void acl_prefetch(prefetch_strategy_t strategy);

void acl_cache(cache_policy_t policy);

void acl_select(char * path);

#endif