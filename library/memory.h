#ifndef MEMORY_H
#define MEMORY_H

#include "cache.h"
#include "structures/queue.h"

struct mapping_entry {
    char me_path[PATH_MAX];
    cache_entry_t * me_entry;
    int me_descriptor;
    int me_flags;
    mode_t me_mode;
};

typedef struct mapping_entry mapping_entry_t;

struct memory {

    cache_t m_cache;

    queue_t m_queue_low;
    queue_t m_queue_high;
    queue_t m_queue_prefetch;

    mapping_entry_t ** m_mapping_entries;
    pthread_mutex_t * m_mapping_mutex;
};

typedef struct memory memory_t;

void init_memory(memory_t * memory);

void free_memory(memory_t * memory);

#endif //MEMORY_H
