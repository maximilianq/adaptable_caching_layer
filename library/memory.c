#include "memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"
#include "calls.h"
#include "constants.h"

#include "utils/file.h"

#include "structures/queue.h"

void inserted_cache(cache_entry_t * entry, void * data) {

    memory_t * memory = data;

    for (int i = 0; i < 1024; i++) {

        pthread_mutex_lock(&memory->m_mapping_mutex[i]);

        mapping_entry_t * mapping_entry = memory->m_mapping_entries[i];

        if (mapping_entry != NULL && mapping_entry->me_descriptor == -1 && strcmp(mapping_entry->me_path, entry->ce_source) == 0) {
            mapping_entry->me_entry = entry;
            mapping_entry->me_descriptor = sys_open(entry->ce_cache, mapping_entry->me_flags, mapping_entry->me_mode);
        }

        pthread_mutex_unlock(&memory->m_mapping_mutex[i]);
    }
}

void removed_cache(cache_entry_t * entry, void * data) {

    memory_t * memory = data;

    for (int i = 0; i < 1024; i++) {

        pthread_mutex_lock(&memory->m_mapping_mutex[i]);

        mapping_entry_t * mapping_entry = memory->m_mapping_entries[i];

        if (mapping_entry != NULL && mapping_entry->me_entry == entry) {

            sys_close(mapping_entry->me_descriptor);

            pthread_mutex_lock(&memory->m_mapping_entries[i]->me_entry->ce_mutex);

            ssize_t upper, lower;
            while (pop_extent(&memory->m_mapping_entries[i]->me_entry->ce_extents, &lower, &upper) != -1) {
                copy_file_interval(memory->m_mapping_entries[i]->me_entry->ce_cache, memory->m_mapping_entries[i]->me_entry->ce_source, lower, upper);
            }

            pthread_mutex_unlock(&memory->m_mapping_entries[i]->me_entry->ce_mutex);

            mapping_entry->me_entry = NULL;
            mapping_entry->me_descriptor = -1;
        }

        pthread_mutex_unlock(&memory->m_mapping_mutex[i]);
    }
}

void init_memory(memory_t * memory) {

    init_cache(&memory->m_cache, CACHE_SIZE, inserted_cache, memory, removed_cache, memory);
    init_queue(&memory->m_queue_low, 4096);
    init_queue(&memory->m_queue_high, 4096);
    init_queue(&memory->m_queue_prefetch, 4096);

    memory->m_mapping_entries = malloc(1024 * sizeof(mapping_entry_t *));
    memory->m_mapping_mutex = malloc(1024 * sizeof(pthread_mutex_t));

    for (int i = 0; i < 1024; i++) {
        memory->m_mapping_entries[i] = NULL;
        pthread_mutex_init(&memory->m_mapping_mutex[i], NULL);
    }
}

void free_memory(memory_t * memory) {

    free_cache(&memory->m_cache);
    free_queue(&memory->m_queue_low);
    free_queue(&memory->m_queue_high);
    free_queue(&memory->m_queue_prefetch);

    for (int i = 0; i < 1024; i++) {

        if (memory->m_mapping_entries[i] != NULL) {
            free(memory->m_mapping_entries[i]);
            memory->m_mapping_entries[i] = NULL;
        }

        pthread_mutex_destroy(&memory->m_mapping_mutex[i]);
    }

    free(memory->m_mapping_entries);
    free(memory->m_mapping_mutex);
}