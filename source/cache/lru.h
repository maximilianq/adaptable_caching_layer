#ifndef LRU_H
#define LRU_H

#include "../cache.h"

void init_lru(cache_t * cache);

void free_lru(cache_t * cache);

void insert_lru(cache_t * cache, cache_entry_t * entry);

cache_entry_t * remove_lru(cache_t * cache, char * path);

cache_entry_t * retrieve_lru(cache_t * cache, char * path);

cache_entry_t * pop_lru(cache_t * cache);

#endif