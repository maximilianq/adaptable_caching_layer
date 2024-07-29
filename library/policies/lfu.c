#include "lfu.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

#include "../calls.h"
#include "../constants.h"
#include "../file.h"
#include "../path.h"
#include "../structures/queue.h"
#include "../structures/heap.h"

int cache_predicate_lfu(const void * data, const void * search) {
    return strcmp(((cache_entry_t *) data)->ce_source, search) == 0;
}

unsigned long cache_transform_lfu(unsigned long key) {
    return key + 1;
}

void cache_init_lfu(cache_t * cache) {
    cache->c_size = 0;
    cache->c_capacity = CACHE_SIZE;
    init_queue(&cache->c_prefetch_entries, 4096);
    init_heap(&cache->c_cache_entries, 4096, cache_transform_lfu, cache_predicate_lfu);
    pthread_mutex_init(&cache->c_mutex_cache, NULL);
    for (int i = 0; i < 1024; i++) pthread_mutex_init(&cache->c_mutex_mapping[i], NULL);
}

void cache_free_lfu(cache_t * cache) {
    free_queue(&cache->c_prefetch_entries);
    free_heap(&cache->c_cache_entries);
    pthread_mutex_destroy(&cache->c_mutex_cache);
    for (int i = 0; i < 1024; i++) pthread_mutex_destroy(&cache->c_mutex_mapping[i]);
}

int cache_access_lfu(cache_t * cache, const char * source_path, char * cache_path) {

    cache_entry_t entry;

    int result = update_heap(&cache->c_cache_entries, source_path, &entry, sizeof(cache_entry_t));
    if (result == -1 || entry.ce_status == 0) {
        return -1;
    }

    struct stat test;
    if (lstat(source_path, &test) == -1) {
        perror("ERROR: could not stat source file!");
        exit(EXIT_FAILURE);
    }

    if(test.st_mtime != entry.ce_mtime) {
        return -1;
    }

    strcpy(cache_path, entry.ce_cache);

    return 0;
}

void cache_update_lfu(cache_t * cache, const char * source_path) {

    // convert source path to cache path
    char cache_path[PATH_MAX];
    if (get_cache_path(source_path, cache_path, PATH_MAX) == NULL) {
        return;
    }

    // retrieve data from source file
    struct stat source_stat;
    if (stat(source_path, &source_stat) == -1) {
        perror("ERROR: could not stat source file!");
        exit(EXIT_FAILURE);
    }

    // create cache entry
    cache_entry_t * entry = malloc(sizeof(cache_entry_t));
    strcpy(entry->ce_source, source_path);
    strcpy(entry->ce_cache, cache_path);
    entry->ce_status = 0;
    entry->ce_size = source_stat.st_size;
    entry->ce_mtime = source_stat.st_mtime;
    pthread_mutex_init(&entry->ce_mutex, NULL);

    // aquire lock on
    pthread_mutex_lock(&cache->c_mutex_cache);

    // update cache with this item
    cache_entry_t added;
    int result = insert_heap(&cache->c_cache_entries, time(NULL), source_path, entry, &added, sizeof(cache_entry_t));
    if (result == -1) {
        perror("ERROR: could not add item to cache!");
        exit(EXIT_FAILURE);
    }

    // if item already in cache replace entity with returned entity
    if (result == 1) {
        pthread_mutex_unlock(&cache->c_mutex_cache);
        free(entry);
    }

    // if item not yet in cache copy file to cache directory
    if (result == 0) {

        // remove items from cache until size is within bounds again
        cache_entry_t remove;
        while (cache->c_size + entry->ce_size > cache->c_capacity) {

            if (extract_heap(&cache->c_cache_entries, &remove, sizeof(cache_entry_t)) == -1) {
                perror("ERROR: could not remove item from cache!");
                exit(EXIT_FAILURE);
            }

            cache->c_size = cache->c_size - remove.ce_size;

            // iterate over mappings and close for each mapped file a cached version
            for (int i = 0; i < 1024; i++) {
                pthread_mutex_lock(&cache->c_mutex_mapping[i]);
                if (cache->c_mapping[i] != NULL && cache->c_mapping[i]->cm_file != -1 && strcmp(remove.ce_source, cache->c_mapping[i]->cm_source) == 0) {
                    sys_close(cache->c_mapping[i]->cm_file);
                    cache->c_mapping[i]->cm_file = -1;
                }
                pthread_mutex_unlock(&cache->c_mutex_mapping[i]);
            }

            if (remove.ce_status == 1 && unlink(remove.ce_cache) == -1) {
                perror("ERROR: could not remove cached file!");
                exit(EXIT_FAILURE);
            }
        }

        // update size of cache
        cache->c_size = cache->c_size + added.ce_size;

        // unlock cache mutex
        pthread_mutex_unlock(&cache->c_mutex_cache);

        // copy file from source to cache
        copy_file(source_path, cache_path);

        // iterate over mappings and open for each mapped file a cached version
        for (int i = 0; i < 1024; i++) {
            pthread_mutex_lock(&cache->c_mutex_mapping[i]);
            if (cache->c_mapping[i] != NULL && cache->c_mapping[i]->cm_file == -1 && strcmp(entry->ce_source, cache->c_mapping[i]->cm_source) == 0) {
                cache->c_mapping[i]->cm_file = sys_open(entry->ce_cache, cache->c_mapping[i]->cm_flags, cache->c_mapping[i]->cm_mode);
            }
            pthread_mutex_unlock(&cache->c_mutex_mapping[i]);
        }

        // set status to in cache
        entry->ce_status = 1;
    }
}