#include "lfu.h"

#include "../structures/lookup.h"
#include "../structures/priority.h"

#include "../utils/string.h"

typedef struct lfu {
    lookup_t f_lookup;
    priority_t f_priority;
} lfu_t;

unsigned long priority_insert_lfu() {
    return 0;
}

unsigned long priority_update_lfu(unsigned long prev) {
    return prev + 1;
}

void init_lfu(cache_t * cache) {

    lfu_t * lfu = malloc(sizeof(lfu_t));
    cache->c_data = lfu;

    init_lookup(&lfu->f_lookup, 4096, hash_string, equal_string);
    init_priority(&lfu->f_priority, 4096, hash_cache_entry, equal_cache_entry, PRIO_MIN, priority_insert_lfu, priority_update_lfu);
}

void free_lfu(cache_t * cache) {

    lfu_t * lfu = cache->c_data;

    free_lookup(&lfu->f_lookup);
    free_priority(&lfu->f_priority);

    free(cache->c_data);
    cache->c_data = NULL;
}

void insert_lfu(cache_t * cache, cache_entry_t * entry) {

    lfu_t * lfu = cache->c_data;

    insert_lookup(&lfu->f_lookup, entry->ce_source, entry);
    insert_priority(&lfu->f_priority, time(NULL), entry);
}

cache_entry_t * remove_lfu(cache_t * cache, char * path) {

    lfu_t * lfu = cache->c_data;

    cache_entry_t * entry = remove_lookup(&lfu->f_lookup, path);
    if (entry == NULL) {
        return NULL;
    }

    remove_priority(&lfu->f_priority, entry);

    return entry;
}

cache_entry_t * retrieve_lfu(cache_t * cache, char * path) {

    lfu_t * lfu = cache->c_data;

    cache_entry_t * entry = retreive_lookup(&lfu->f_lookup, path);
    if (entry == NULL) {
        return NULL;
    }

    update_priority(&lfu->f_priority, time(NULL), entry);

    return entry;
}

cache_entry_t * pop_lfu(cache_t * cache) {

    lfu_t * lfu = cache->c_data;

    cache_entry_t * entry = pop_priority(&lfu->f_priority);
    if (entry == NULL) {
        return NULL;
    }

    remove_lookup(&lfu->f_lookup, entry->ce_source);

    return entry;
}