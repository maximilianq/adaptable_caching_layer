#include "fsdl.h"

#include <stdio.h>
#include <string.h>

#include "../utils/path.h"
#include "../utils/directory.h"

#include <sys/stat.h>

#include "../prefetch.h"
#include "../structures/markov.h"

typedef struct fsdl_state {
    char directory[PATH_MAX];
    char ** entries;
    int count;
    int offset;
} fsdl_state_t;

void init_fsdl(prefetch_t * prefetch) {
    prefetch->p_data = malloc(sizeof(fsdl_state_t));

    fsdl_state_t * state = prefetch->p_data;
    state->entries = NULL;
    state->count = 0;
    state->offset = 0;
}

void free_fsdl(prefetch_t * prefetch) {
    free(((fsdl_state_t *) prefetch->p_data)->directory);
    free(((fsdl_state_t *) prefetch->p_data)->entries);
}

void process_fsdl(prefetch_t * prefetch, char * source_path) {

    fsdl_state_t * fsdl_state = prefetch->p_data;

    // retrieve parent path of current file
    char parent_path[PATH_MAX];
    get_parent_path(source_path, parent_path, PATH_MAX);

    if (fsdl_state->offset == fsdl_state->count || strcmp(parent_path, fsdl_state->directory) != 0) {

        strcpy(fsdl_state->directory, parent_path);

        list_files(parent_path, &fsdl_state->entries, &fsdl_state->count);

        // sort files according to natural order
        qsort(fsdl_state->entries, fsdl_state->count, sizeof(char *), compare_path);

        fsdl_state->offset = 0;
    }

    prefetch_predict(prefetch, fsdl_state->entries[fsdl_state->offset]);
    free(fsdl_state->entries[fsdl_state->offset]);

    fsdl_state->offset += 1;
}