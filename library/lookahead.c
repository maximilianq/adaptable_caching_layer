#define _GNU_SOURCE

#include "lookahead.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>

#include "file.h"
#include "path.h"
#include "queue.h"
#include "cache.h"

cache_t * cache_lookahead;

void process_lookahead(char * path) {

    char parent_path[PATH_MAX], source_path[PATH_MAX];

    get_parent_path(path, parent_path, PATH_MAX);

    DIR * source_directory = opendir(parent_path);
    struct dirent * source_entry;

    while ((source_entry = readdir(source_directory)) != NULL) {

        // assemble absolute path of file
        strcpy(source_path, parent_path);
        strcat(source_path, "/");
        strcat(source_path, source_entry->d_name);

        // get stat from source file
        struct stat source_stat;
        if (lstat(source_path, &source_stat) == -1) {
            perror("ERROR: could not stat source file!\n");
            exit(EXIT_FAILURE);
        }

        if (!S_ISREG(source_stat.st_mode)) {
            continue;
        }

        cache_update(cache_lookahead, source_path);
    }

    closedir(source_directory);
}

void * handle_lookahead(void * data) {

    cache_lookahead = (cache_t *) data;

    char * value;
    while((value = (char *) dequeue(&cache_lookahead->c_lookahead))) {
        process_lookahead(value);
    }

    pthread_exit(NULL);
}