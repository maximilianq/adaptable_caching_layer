#include "cache.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

#include "calls.h"
#include "file.h"
#include "path.h"
#include "constants.h"
#include "queue.h"

void cache_init(cache_t * cache) {

    cache->c_head = NULL;
    cache->c_tail = NULL;

    cache->c_entries = 0;
    cache->c_size = 0;
    cache->c_capacity = CACHE_SIZE;

    init_queue(&cache->c_lookahead, 4096);

    pthread_mutex_init(&cache->c_mutex_cache, NULL);
    for (int i = 0; i < 1024; i++) pthread_mutex_init(&cache->c_mutex_mapping[i], NULL);
}

void cache_free(cache_t * cache) {

    free_queue(&cache->c_lookahead);

    pthread_mutex_destroy(&cache->c_mutex_cache);
    for (int i = 0; i < 1024; i++) pthread_mutex_destroy(&cache->c_mutex_mapping[i]);
}

char * cache_query(cache_t * cache, const char * source_path) {

    pthread_mutex_lock(&cache->c_mutex_cache);

    // search for the current entry in cache
    cache_entry_t * entry = cache->c_head;
    while (entry != NULL) {
        if (strcmp(source_path, entry->ce_source) == 0) {
            break;
        }
        entry = entry->ce_next;
    }

    // if entry found update c_recency
    if (entry != NULL) {

        // unlink entry from current position
        if (entry->ce_previos != NULL) entry->ce_previos->ce_next = entry->ce_next;
        else cache->c_head = entry->ce_next;
        if (entry->ce_next != NULL) entry->ce_next->ce_previos = entry->ce_previos;
        else cache->c_tail = entry->ce_previos;

        // update entry
        entry->ce_next = cache->c_head;
        entry->ce_previos = NULL;

        entry->ce_recency = time(NULL);

        // link at beginning
        if (cache->c_head != NULL) cache->c_head->ce_previos = entry;
        cache->c_head = entry;
        if (cache->c_tail == NULL) cache->c_tail = entry;
    }

    if (entry != NULL && !entry->ce_ready) {
        entry = NULL;
    }

    pthread_mutex_unlock(&cache->c_mutex_cache);

    return entry == NULL ? NULL : entry->ce_cache;
}

void cache_update(cache_t * cache, const char * source_path) {

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

    pthread_mutex_lock(&cache->c_mutex_cache);

    // search for the current file in cache starting from the latest accessed one
    cache_entry_t * entry = cache->c_head;
    while (entry != NULL) {
        if (strcmp(source_path, entry->ce_source) == 0) {
            break;
        }
        entry = entry->ce_next;
    }

    // if entry found update c_recency and reorder list
    if (entry != NULL) {

        // unlink entry from current position
        if (entry->ce_previos != NULL) entry->ce_previos->ce_next = entry->ce_next;
        else cache->c_head = entry->ce_next;
        if (entry->ce_next != NULL) entry->ce_next->ce_previos = entry->ce_previos;
        else cache->c_tail = entry->ce_previos;

        // update entry
        entry->ce_next = cache->c_head;
        entry->ce_previos = NULL;

        entry->ce_recency = time(NULL);

        // link at beginning
        if (cache->c_head != NULL) cache->c_head->ce_previos = entry;
        cache->c_head = entry;
        if (cache->c_tail == NULL) cache->c_tail = entry;

        pthread_mutex_unlock(&cache->c_mutex_cache);

        // retrieve data from cache file to update in case of source file change
        struct stat cache_stat;
        if (stat(cache_path, &cache_stat) == -1) {
            perror("ERROR: could not stat cache file!");
            exit(EXIT_FAILURE);
        }

        // compare source and cache mtime to avoid overwriting a newer or same file
        if (cache_stat.st_mtime < source_stat.st_mtime) {
            copy_file(entry->ce_source, entry->ce_cache);
        }
    }

    // if entry not found clear older items from cache and add current file
    else {

        // delete the required amount of files from cache starting from the oldest one
        while (cache->c_size + source_stat.st_size > cache->c_capacity) {

            // get last entry
            cache_entry_t * last = cache->c_tail;

            // unlink last entry
            if (last->ce_previos != NULL) last->ce_previos->ce_next = NULL;
            cache->c_tail = last->ce_previos;
            if (cache->c_tail == NULL) cache->c_head = NULL;

            // update cache stats
            cache->c_entries--;
            cache->c_size -= last->ce_size;

            for (int i = 0; i < 1024; i++) {
                pthread_mutex_lock(&cache->c_mutex_mapping[i]);
                if (cache->c_mapping[i] != NULL) {
                    if (cache->c_mapping[i]->cm_file != -1 && strcmp(cache->c_mapping[i]->cm_path, last->ce_source) == 0) {
                        if (sys_close(cache->c_mapping[i]->cm_file) == -1) {
                            perror("ERROR: could not close cache path!");
                            exit(EXIT_FAILURE);
                        }
                        cache->c_mapping[i]->cm_file = -1;
                    }
                }
                pthread_mutex_unlock(&cache->c_mutex_mapping[i]);
            }

            if (unlink(last->ce_cache) == -1) {
                perror("ERROR: could not remove file from cache!");
                exit(EXIT_FAILURE);
            }

            free(last);
        }

        // create new entry
        entry = malloc(sizeof(cache_entry_t));

        // initialize new entry
        strcpy(entry->ce_source, source_path);
        strcpy(entry->ce_cache, cache_path);
        entry->ce_size = source_stat.st_size;

        entry->ce_ready = false;
        entry->ce_recency = time(NULL);

        entry->ce_previos = NULL;
        entry->ce_next = cache->c_head;

        // link at beginning
        if (cache->c_head != NULL) cache->c_head->ce_previos = entry;
        cache->c_head = entry;
        if (cache->c_tail == NULL) cache->c_tail = entry;

        // update cache stats
        cache->c_entries++;
        cache->c_size += entry->ce_size;

        pthread_mutex_unlock(&cache->c_mutex_cache);

        // copy file from source to cache

        copy_file(entry->ce_source, entry->ce_cache);

        // set times of source file to keep track of changes of underlaying file
        struct utimbuf times;
        times.actime = source_stat.st_atime;
        times.modtime = source_stat.st_mtime;
        if (utime(cache_path, &times) == -1) {
            perror("ERROR: could not set utime of cache file");
            exit(EXIT_FAILURE);
        }

        // update already open mappings
        for (int i = 0; i < 1024; i++) {
            pthread_mutex_lock(&cache->c_mutex_mapping[i]);
            if (cache->c_mapping[i] != NULL) {
                if (cache->c_mapping[i]->cm_file == -1 && strcmp(cache->c_mapping[i]->cm_path, source_path) == 0) {
                    cache->c_mapping[i]->cm_file = sys_open(cache_path, cache->c_mapping[i]->cm_mode, 0);
                }
            }
	        pthread_mutex_unlock(&cache->c_mutex_mapping[i]);
        }

        entry->ce_ready = true;
    }
}