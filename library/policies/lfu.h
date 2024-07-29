#ifndef CACHE_LFU_H
#define CACHE_LFU_H

#include "../cache.h"

void cache_init_lfu(cache_t * cache);

void cache_free_lfu(cache_t * cache);

int cache_access_lfu(cache_t * cache, const char * source_path, char * cache_path);

void cache_update_lfu(cache_t * cache, const char * source_path);

#endif