#include "lru.h"

#include "../structures/lookup.h"
#include "../structures/priority.h"

#include "../utils/string.h"

typedef struct lru {
    lookup_t f_lookup;
    priority_t f_priority;
} lru_t;

unsigned long priority_insert_lru() {
    return time(NULL);
}

unsigned long priority_update_lru(unsigned long prev) {
    return time(NULL);
}

void init_lru(cache_t * cache) {

    lru_t * lru = malloc(sizeof(lru_t));
    cache->c_data = lru;

    init_lookup(&lru->f_lookup, 4096, hash_string, equal_string);
    init_priority(&lru->f_priority, 4096, hash_cache_entry, equal_cache_entry, PRIO_MIN, priority_insert_lru, priority_update_lru);
}

void free_lru(cache_t * cache) {

    lru_t * lru = cache->c_data;

    free_lookup(&lru->f_lookup);
    free_priority(&lru->f_priority);

    free(cache->c_data);
    cache->c_data = NULL;
}

void insert_lru(cache_t * cache, cache_entry_t * entry) {

    lru_t * lru = cache->c_data;

    insert_lookup(&lru->f_lookup, entry->ce_source, entry);
    insert_priority(&lru->f_priority, time(NULL), entry);
}

cache_entry_t * remove_lru(cache_t * cache, char * path) {

    lru_t * lru = cache->c_data;

    cache_entry_t * entry = remove_lookup(&lru->f_lookup, path);
    if (entry == NULL) {
        return NULL;
    }

    remove_priority(&lru->f_priority, entry);

    return entry;
}

cache_entry_t * retrieve_lru(cache_t * cache, char * path) {

    lru_t * lru = cache->c_data;

    cache_entry_t * entry = retreive_lookup(&lru->f_lookup, path);
    if (entry == NULL) {
        return NULL;
    }

    update_priority(&lru->f_priority, time(NULL), entry);

    return entry;
}

cache_entry_t * pop_lru(cache_t * cache) {

    lru_t * lru = cache->c_data;

    cache_entry_t * entry = pop_priority(&lru->f_priority);
    if (entry == NULL) {
        return NULL;
    }

    remove_lookup(&lru->f_lookup, entry->ce_source);

    return entry;
}