#ifndef FIFO_H
#define FIFO_H

#include "../cache.h"

void init_fifo(cache_t * cache);

void free_fifo(cache_t * cache);

void insert_fifo(cache_t * cache, cache_entry_t * entry);

cache_entry_t * remove_fifo(cache_t * cache, char * path);

cache_entry_t * retrieve_fifo(cache_t * cache, char * path);

cache_entry_t * pop_fifo(cache_t * cache);

#endif