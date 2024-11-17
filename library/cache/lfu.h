#ifndef LFU_H
#define LFU_H

#include "../cache.h"

void init_lfu(cache_t * cache);

void free_lfu(cache_t * cache);

void insert_lfu(cache_t * cache, cache_entry_t * entry);

cache_entry_t * remove_lfu(cache_t * cache, char * path);

cache_entry_t * retrieve_lfu(cache_t * cache, char * path);

cache_entry_t * pop_lfu(cache_t * cache);

#endif