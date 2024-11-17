#ifndef ACL_H
#define ACL_H

typedef struct prefetch prefetch_t;

// functions to be implemented by custom prefetching logic
typedef void (* init_prefetch_t) (prefetch_t * memory);
typedef void (* free_prefetch_t) (prefetch_t * memory);
typedef void (* process_prefetch_t) (prefetch_t * memory, char * path);

typedef struct prefetch_strategy {
    init_prefetch_t ps_init;
    free_prefetch_t ps_free;
    process_prefetch_t ps_process;
} prefetch_strategy_t;

int ACL_WILLNEED = 1;
int ACL_DONTNEED = 2;
int acl_advise(const char * path, int flags);

void acl_prefetch(prefetch_strategy_t strategy);

void acl_predict(char * path);

void acl_cache(prefetch_strategy_t strategy);

#endif