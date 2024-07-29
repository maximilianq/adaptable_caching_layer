#ifndef CACHE_LRU_H
#define CACHE_LRU_H

#include "../cache.h"

void cache_init_lru(cache_t * cache);

void cache_free_lru(cache_t * cache);

int cache_access_lru(cache_t * cache, const char * source_path, char * cache_path);

void cache_update_lru(cache_t * cache, const char * source_path);

#endif