#ifndef CACHE_VARIANT_H
#define CACHE_VARIANT_H

#include "fifo.h"
cache_policy_t policy_fifo = {
    .cp_init = init_fifo,
    .cp_free = free_fifo,
    .cp_insert = insert_fifo,
    .cp_remove = remove_fifo,
    .cp_retrieve = retrieve_fifo,
    .cp_pop = pop_fifo
};

#include "lfu.h"
cache_policy_t policy_lfu = {
    .cp_init = init_lfu,
    .cp_free = free_lfu,
    .cp_insert = insert_lfu,
    .cp_remove = remove_lfu,
    .cp_retrieve = retrieve_lfu,
    .cp_pop = pop_lfu
};

#include "lru.h"
cache_policy_t policy_lru = {
    .cp_init = init_lru,
    .cp_free = free_lru,
    .cp_insert = insert_lru,
    .cp_remove = remove_lru,
    .cp_retrieve = retrieve_lru,
    .cp_pop = pop_lru
};

#endif