#include "cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "structures/heap.h"
#include "utils/file.h"
#include "utils/path.h"

#include "structures/priority.h"

int hash_string(void * data) {

    char * pointer = data;
    int output = 0;

    char character;
    while ((character = *pointer++) != '\0') {
        output += character;
    }

    return output;
}

int hash_cache_entry(void * data) {

    char * pointer = ((cache_entry_t *) data)->ce_source;
    int output = 0;

    char character;
    while ((character = *pointer++) != '\0') {
        output += character;
    }

    return output;
}

int equal_string(void * first, void * second) {
    return strcmp(first, second) == 0;
}

int equal_cache_entry(void * first, void * second) {
    return strcmp(((cache_entry_t *) first)->ce_source, ((cache_entry_t *) second)->ce_source) == 0;
}

void init_cache(cache_t * cache, ssize_t capacity, inserted_t inserted, void * inserted_args, removed_t removed, void * removed_args, insert_t insert, update_t update) {

    cache->c_capacity = capacity;
    cache->c_size = 0;

    init_lookup(&cache->c_lookup, 4096, hash_string, equal_string);
    init_priority(&cache->c_priority, 4096, hash_cache_entry, equal_cache_entry, PRIO_MIN, insert, update);

    cache->c_inserted = inserted;
    cache->c_inserted_args = inserted_args;

    cache->c_removed = removed;
    cache->c_removed_args = removed_args;

    pthread_mutex_init(&cache->c_mutex, NULL);
}

void free_cache(cache_t * cache) {
    free_priority(&cache->c_priority);
    free_lookup(&cache->c_lookup);
    pthread_mutex_destroy(&cache->c_mutex);
}

int insert_cache(cache_t * cache, char * path) {

    // expand path of given path
    char source_path[PATH_MAX];
    get_full_path(path, source_path, PATH_MAX);

    // get cache path from the given
    char cache_path[PATH_MAX];
    if (get_cache_path(source_path, cache_path, PATH_MAX) == NULL) {
        return -1;
    }

    struct stat source_stat;
    if (stat(source_path, &source_stat) == -1) {
        perror("ERROR: could not stat source file!");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&cache->c_mutex);

    if (retreive_lookup(&cache->c_lookup, source_path) != NULL) {
        pthread_mutex_unlock(&cache->c_mutex);
        return 1;
    }

    // remove entries until size is small enough to insert entry
    while (cache->c_size + source_stat.st_size > cache->c_capacity) {
        pop_cache(cache);
    }

    cache_entry_t * entry = malloc(sizeof(cache_entry_t));
    strcpy(entry->ce_source, source_path);
    strcpy(entry->ce_cache, cache_path);
    entry->ce_size = source_stat.st_size;
    entry->ce_mtime = source_stat.st_mtime;
    init_extent(&entry->ce_extents);
    pthread_mutex_init(&entry->ce_mutex, NULL);

    pthread_mutex_lock(&entry->ce_mutex);

    // insert entry into lookup table for fast path based access
    insert_lookup(&cache->c_lookup, entry->ce_source, entry);

    // insert entry into priority queue for priority based cache deletion
    insert_priority(&cache->c_priority, time(NULL), entry);

    // update cache size based on file size
    cache->c_size = cache->c_size + entry->ce_size;

    pthread_mutex_unlock(&cache->c_mutex);

    copy_file(entry->ce_source, entry->ce_cache);

    pthread_mutex_unlock(&entry->ce_mutex);

    if (cache->c_inserted != NULL) cache->c_inserted(entry, cache->c_inserted_args);

    return 0;
}

cache_entry_t * retrieve_cache(cache_t * cache, char * path) {

    pthread_mutex_lock(&cache->c_mutex);

    cache_entry_t * entry;
    if ((entry = retreive_lookup(&cache->c_lookup, path)) == NULL) {
        pthread_mutex_unlock(&cache->c_mutex);
        return NULL;
    }

    update_priority(&cache->c_priority, time(NULL), entry);

    pthread_mutex_unlock(&cache->c_mutex);

    return entry;
}

int remove_cache(cache_t * cache, char * path) {

    cache_entry_t * entry = remove_lookup(&cache->c_lookup, path);
    if (entry == NULL) {
        return -1;
    }

    remove_priority(&cache->c_priority, entry);

    // adapt size of cache
    cache->c_size = cache->c_size - entry->ce_size;

    if (cache->c_removed != NULL) cache->c_removed(entry, cache->c_removed_args);

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

int pop_cache(cache_t * cache) {

    // retrieve entry to be deleted from priority queue
    cache_entry_t * entry;
    if ((entry = pop_priority(&cache->c_priority)) == NULL) {
        return -1;
    }

    // remove entry from lookup table
    remove_lookup(&cache->c_lookup, entry->ce_source);

    // adapt size of cache
    cache->c_size = cache->c_size - entry->ce_size;

    if (cache->c_removed != NULL) cache->c_removed(entry, cache->c_removed_args);

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