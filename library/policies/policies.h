#ifndef POLICIES_H

    #define POLICIES_H

    #include "../constants.h"

    #if defined(CACHE_POLICY_LRU)
        #include "lru.h"
        #define cache_init cache_init_lru
        #define cache_free cache_free_lru
        #define cache_access cache_access_lru
        #define cache_update cache_update_lru
    #elif defined(CACHE_POLICY_LFU)
        #include "lfu.h"
        #define cache_init cache_init_lfu
        #define cache_free cache_free_lfu
        #define cache_access cache_access_lfu
        #define cache_update cache_update_lfu
    #endif

#endif