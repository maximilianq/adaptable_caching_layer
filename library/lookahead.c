#define _GNU_SOURCE

#include "lookahead.h"

#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>

#include "file.h"
#include "path.h"
#include "queue.h"
#include "operations.h"
#include "calls.h"

cache_t * cache_lookahead;

void process_lookahead(char * path) {

    struct dirent * source_entry;
    char parent_path[PATH_MAX], source_path[PATH_MAX], cache_path[PATH_MAX];
    struct stat source_stat;
    time_t mtime;

    get_parent_path(path, parent_path, PATH_MAX);

    DIR * source_directory = opendir(parent_path);

    while ((source_entry = readdir(source_directory)) != NULL) {

        // assemble absolute path of file
        strcpy(source_path, parent_path);
        strcat(source_path, "/");
        strcat(source_path, source_entry->d_name);

        // get stat of source path and continue if not found or not regular file
        if (stat(source_path, &source_stat) == -1 || !S_ISREG(source_stat.st_mode)) {
            continue;
        }

        // get cache path of file
        if (get_cache_path(source_path, cache_path, PATH_MAX) == NULL) {
            continue;
        }

        // get mtime of cached file and continue if already present
        if (getxattr(cache_path, "user.mtime", &mtime, sizeof(mtime)) != -1 && mtime == source_stat.st_mtime) {
            continue;
        }

        // copy file to cache
        copy_file(source_path, cache_path);

        // set mtime to cached file using mtime of cached file
        if (setxattr(cache_path, "user.mtime", &source_stat.st_mtime, sizeof(source_stat.st_mtime), 0) == -1) {
            perror("ERROR: could not set xattr to cache file!\n");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 4096; i++) {

            pthread_mutex_lock(&cache_lookahead->c_items_mutex[i]);

            if (cache_lookahead->c_items[i] != NULL) {
                if(strcmp(cache_lookahead->c_items[i]->c_path, cache_path)) {
                    cache_lookahead->c_items[i]->c_file = sys_open(cache_path, cache_lookahead->c_items[i]->c_flags, 0);
                }
            }

	        pthread_mutex_unlock(&cache_lookahead->c_items_mutex[i]);
        }
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