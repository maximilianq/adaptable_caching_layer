
#include "mapping.h"

#include <string.h>

#include "calls.h"
#include "utils/file.h"

void mapping_init(mapping_t * mapping) {

    mapping->m_entries = malloc(1024 * sizeof(mapping_entry_t *));
    mapping->m_mutex = malloc(1024 * sizeof(pthread_mutex_t));

    for (int i = 0; i < 1024; i++) {
        mapping->m_entries[i] = NULL;
        pthread_mutex_init(&mapping->m_mutex[i], NULL);
    }
}

void mapping_free(mapping_t * mapping) {

    for (int i = 0; i < 1024; i++) {

        if (mapping->m_entries[i] != NULL) {
            free(mapping->m_entries[i]);
            mapping->m_entries[i] = NULL;
        }

        pthread_mutex_destroy(&mapping->m_mutex[i]);
    }

    free(mapping->m_entries);
    free(mapping->m_mutex);
}

void mapping_cache_inserted(cache_entry_t * entry, void * data) {

    mapping_t * mapping = data;

    for (int i = 0; i < 1024; i++) {

        pthread_mutex_lock(&mapping->m_mutex[i]);

        mapping_entry_t * mapping_entry = mapping->m_entries[i];

        if (mapping_entry != NULL && mapping_entry->me_descriptor == -1 && strcmp(mapping_entry->me_path, entry->ce_source) == 0) {
            mapping_entry->me_entry = entry;
            mapping_entry->me_descriptor = sys_open(entry->ce_cache, mapping_entry->me_flags, mapping_entry->me_mode);
        }

        pthread_mutex_unlock(&mapping->m_mutex[i]);
    }
}

void mapping_cache_removed(cache_entry_t * entry, void * data) {

    mapping_t * mapping = data;

    for (int i = 0; i < 1024; i++) {

        pthread_mutex_lock(&mapping->m_mutex[i]);

        mapping_entry_t * mapping_entry = mapping->m_entries[i];

        if (mapping_entry != NULL && mapping_entry->me_entry == entry) {

            sys_close(mapping_entry->me_descriptor);

            pthread_mutex_lock(&mapping->m_entries[i]->me_entry->ce_mutex);

            ssize_t upper, lower;
            while (pop_extent(&mapping->m_entries[i]->me_entry->ce_extents, &lower, &upper) != -1) {
                copy_file_interval(mapping->m_entries[i]->me_entry->ce_cache, mapping->m_entries[i]->me_entry->ce_source, lower, upper);
            }

            pthread_mutex_unlock(&mapping->m_entries[i]->me_entry->ce_mutex);

            mapping_entry->me_entry = NULL;
            mapping_entry->me_descriptor = -1;
        }

        pthread_mutex_unlock(&mapping->m_mutex[i]);
    }
}
