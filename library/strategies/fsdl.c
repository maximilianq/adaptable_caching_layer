#include "fsdl.h"

#include <stdio.h>
#include <stdlib.h>

#include "../directory.h"
#include "../path.h"

#include "../structures/queue.h"

#include "../policies/policies.h"

void process_fsdl(cache_t * cache, char * source_path) {

    // retrieve parent path of current file
    char parent_path[PATH_MAX];
    get_parent_path(source_path, parent_path, PATH_MAX);

    // retrieve files from directory
    char ** entries_buffer;
    int entries_size;
    list_files(parent_path, &entries_buffer, &entries_size);

    // sort files according to natural order
    qsort(entries_buffer, entries_size, sizeof(char *), compare_path);

    // update cache with found files sequentially
    for (int i = 0; i < entries_size; i++) {
        cache_update(cache, entries_buffer[i]);
    }
}

void * handle_fsdl(void * data) {

    cache_t * cache = data;

    char * value;
    while((value = (char *) dequeue(&cache->c_prefetch_entries))) {
        process_fsdl(cache, value);
    }

    pthread_exit(NULL);
}