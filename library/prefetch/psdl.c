#include "fsdl.h"

#include <stdio.h>
#include <string.h>

#include "../utils/path.h"
#include "../utils/directory.h"

void process_fsdl(prefetch_t * prefetch, char * source_path) {

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
        prefetch_predict(prefetch, entries_buffer[i]);
        free(entries_buffer[i]);
    }

    free(entries_buffer);
}