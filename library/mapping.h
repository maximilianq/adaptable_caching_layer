#ifndef MAPPING_H
#define MAPPING_H

#include <linux/limits.h>

#include "cache.h"

typedef struct mapping_entry {
    char me_path[PATH_MAX];
    cache_entry_t * me_entry;
    int me_descriptor;
    int me_flags;
    mode_t me_mode;
    int me_hinted;
} mapping_entry_t;

typedef struct mapping {
    mapping_entry_t ** m_entries;
    pthread_mutex_t * m_mutex;
} mapping_t;

void mapping_init(mapping_t * mapping);

void mapping_free(mapping_t * mapping);

void mapping_cache_inserted(cache_entry_t * entry, void * data);

void mapping_cache_removed(cache_entry_t * entry, void * data);

#endif
