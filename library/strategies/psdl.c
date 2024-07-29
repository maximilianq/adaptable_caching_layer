#include "psdl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../directory.h"
#include "../path.h"

#include "../structures/queue.h"

#include "../policies/policies.h"

void process_psdl(cache_t * cache, char * source_path) {

    // retrieve parent path of current file
    char parent_path[PATH_MAX];
    get_parent_path(source_path, parent_path, PATH_MAX);

    // retrieve files from directory
    char ** entries_buffer;
    int entries_size;
    list_files(parent_path, &entries_buffer, &entries_size);

    // sort files according to natural order
    qsort(entries_buffer, entries_size, sizeof(char *), compare_path);

    int index = 0;
    while (index < entries_size && strcmp(source_path, entries_buffer[index]) != 0) {
        index = index + 1;
    }

    // update cache with found files sequentially
    int count = 0;
    while ((index + count) < entries_size && count < 5) {
        cache_update(cache, entries_buffer[index + count]);
        count = count + 1;
    }
}

void * handle_psdl(void * data) {

    cache_t * cache = data;

    char * value;
    while((value = (char *) dequeue(&cache->c_prefetch_entries))) {
        process_psdl(cache, value);
    }

    pthread_exit(NULL);
}