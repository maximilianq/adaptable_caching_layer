#include "prdl.h"

#include <stdio.h>
#include <string.h>

#include "../cache.h"
#include "../state.h"

#include "../utils/directory.h"
#include "../utils/path.h"

#include "../structures/queue.h"

void process_prdl(state_t * state, char * source_path) {

    // retrieve parent path of current file
    char parent_path[PATH_MAX];
    get_parent_path(source_path, parent_path, PATH_MAX);

    // retrieve files from directory
    char ** entries_buffer;
    int entries_size;
    list_files(parent_path, &entries_buffer, &entries_size);

    // search for current entry sequentially
    int index = 0;
    while (index < entries_size && strcmp(source_path, entries_buffer[index]) != 0) {
        index = index + 1;
    }

    // update cache with found files sequentially
    for (int i = index; i < entries_size && i < index + 5; i++) {

        char * path = malloc(PATH_MAX * sizeof(char));
        strcpy(path, entries_buffer[i]);

        enqueue(&state->m_queue_low, path);

        free(entries_buffer[i]);
    }

    free(entries_buffer);
}

void * handle_prdl(void * data) {

    state_t * state = data;

    char * value;
    while(1) {

        if ((value = dequeue(&state->m_queue_high)) != NULL || (value = dequeue(&state->m_queue_low)) != NULL) {
            insert_cache(&state->m_cache, value);
            free(value);
            continue;
        }

        value = dequeue(&state->m_queue_prefetch);
        if (value != NULL) {
            process_prdl(state, value);
            free(value);
        }
    }
}