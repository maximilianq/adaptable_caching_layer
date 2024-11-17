#include "cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "prefetch.h"
#include "utils/file.h"
#include "utils/path.h"

int hash_cache_entry(void * data) {

    char * pointer = ((cache_entry_t *) data)->ce_source;
    int output = 0;

    char character;
    while ((character = *pointer++) != '\0') {
        output += character;
    }

    return output;
}

int equal_cache_entry(void * first, void * second) {
    return strcmp(((cache_entry_t *) first)->ce_source, ((cache_entry_t *) second)->ce_source) == 0;
}

void init_cache(cache_t * cache, mapping_t * mapping, ssize_t capacity, cache_policy_t cache_policy, inserted_cache_t inserted, removed_cache_t removed) {

    cache->c_capacity = capacity;
    cache->c_size = 0;
    cache->c_mapping = mapping;
    cache->c_policy = cache_policy;
    cache->c_inserted = inserted;
    cache->c_removed = removed;

    cache->c_policy.cp_init(cache);

    pthread_mutex_init(&cache->c_mutex, NULL);
}

void free_cache(cache_t * cache) {
    cache->c_policy.cp_free(cache);
    pthread_mutex_destroy(&cache->c_mutex);
}

void cache_replace(cache_t * cache, cache_policy_t cache_policy) {

    pthread_mutex_lock(&cache->c_mutex);

    // free additional storage allocated by the previous caching policy
    cache->c_policy.cp_free(cache);

    // replace caching policy
    cache->c_policy = cache_policy;

    // allocate additional storage allocated for the new caching policy
    cache->c_policy.cp_init(cache);

    pthread_mutex_unlock(&cache->c_mutex);
}

int insert_cache(cache_t * cache, char * path) {

    // get cache path from the given
    char cache_path[PATH_MAX];
    if (get_cache_path(path, cache_path, PATH_MAX) == NULL) {
        return -1;
    }

    struct stat source_stat;
    if (stat(path, &source_stat) == -1) {
        perror("ERROR: could not stat source file!");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&cache->c_mutex);

    cache_entry_t * entry = cache->c_policy.cp_retrieve(cache, path);
    if (entry != NULL) {
        pthread_mutex_unlock(&cache->c_mutex);
        return 1;
    }

    // remove entries until size is small enough to insert entry
    while (cache->c_size + source_stat.st_size > cache->c_capacity) {
        pop_cache(cache);
    }

    entry = malloc(sizeof(cache_entry_t));
    strcpy(entry->ce_source, path);
    strcpy(entry->ce_cache, cache_path);
    entry->ce_size = source_stat.st_size;
    entry->ce_mtime = source_stat.st_mtime;
    init_extent(&entry->ce_extents);
    pthread_mutex_init(&entry->ce_mutex, NULL);

    pthread_mutex_lock(&entry->ce_mutex);

    cache->c_policy.cp_insert(cache, entry);
    cache->c_size = cache->c_size + entry->ce_size;

    pthread_mutex_unlock(&cache->c_mutex);

    struct stat cache_stat;
    if (lstat(entry->ce_cache, &cache_stat) == -1) {
        if (errno != ENOENT) {
            printf("ERROR: could not stat cache file");
            exit(EXIT_FAILURE);
        }
        copy_file(entry->ce_source, entry->ce_cache);
    } else {   
        if (cache_stat.st_size != source_stat.st_size || cache_stat.st_mtime < source_stat.st_mtime) {      
            copy_file(entry->ce_source, entry->ce_cache);
        }
    }

    pthread_mutex_unlock(&entry->ce_mutex);

    if (cache->c_inserted != NULL) cache->c_inserted(cache, entry);

    return 0;
}

cache_entry_t * retrieve_cache(cache_t * cache, char * path) {
    return cache->c_policy.cp_retrieve(cache, path);
}

int remove_cache(cache_t * cache, char * path) {

    pthread_mutex_lock(&cache->c_mutex);

    cache_entry_t * entry = cache->c_policy.cp_remove(cache, path);
    if (entry == NULL) {
        pthread_mutex_unlock(&cache->c_mutex);
        return -1;
    }

    cache->c_size = cache->c_size - entry->ce_size;

    if (cache->c_removed != NULL) cache->c_removed(cache, entry);

    pthread_mutex_unlock(&cache->c_mutex);

    pthread_mutex_lock(&entry->ce_mutex);

    if (unlink(entry->ce_cache) == -1) {
        perror("ERROR: could not delete cached file!");
        exit(EXIT_FAILURE);
    }

    // clean up storage occupied by removed entry
    free_extent(&entry->ce_extents);
    pthread_mutex_destroy(&entry->ce_mutex);

    free(entry);

    return 0;
}

int pop_cache(cache_t * cache) {

    // retrieve entry to be deleted from priority queue
    cache_entry_t * entry = cache->c_policy.cp_pop(cache);
    if (entry == NULL) {
        return -1;
    }
    cache->c_size = cache->c_size - entry->ce_size;

    if (cache->c_removed != NULL) cache->c_removed(cache, entry);

    pthread_mutex_lock(&entry->ce_mutex);

    if (unlink(entry->ce_cache) == -1) {
        perror("ERROR: could not delete cached file!");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&entry->ce_mutex);

    // clean up storage occupied by removed entry
    free_extent(&entry->ce_extents);
    pthread_mutex_destroy(&entry->ce_mutex);

    free(entry);

    return 0;
}