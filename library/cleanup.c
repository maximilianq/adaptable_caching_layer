#define _GNU_SOURCE

#include "cleanup.h"

#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "calls.h"
#include "operations.h"

#define CACHE_PATH "/var/cache/iocache"

cache_t * cache_cleanup;

void process_cleanup() {

    struct dirent * source_entry;
    char cache_path[PATH_MAX];
    struct stat cache_stat;
    time_t current_time = time(NULL);

    DIR * directory = opendir(CACHE_PATH);

    while ((source_entry = readdir(directory)) != NULL) {
        strcpy(cache_path, CACHE_PATH);
        strcat(cache_path, "/");
        strcat(cache_path, source_entry->d_name);

        if (stat(cache_path, &cache_stat) == -1 || !S_ISREG(cache_stat.st_mode)) {
            continue;
        }

        if (cache_stat.st_atime < current_time - 60) {

            for (int i = 0; i < 4096; i++) {

                pthread_mutex_lock(&cache_cleanup->c_items_mutex[i]);

                if (cache_cleanup->c_items[i] != NULL) {
                    if(strcmp(cache_cleanup->c_items[i]->c_path, cache_path)) {
                        cache_cleanup->c_items[i]->c_file = sys_close(cache_cleanup->c_items[i]->c_file);
                        cache_cleanup->c_items[i]->c_file = -1;
                    }
                }

                pthread_mutex_unlock(&cache_cleanup->c_items_mutex[i]);
            }

            if (unlink(cache_path) == -1) {
                perror("ERROR: could not remove file from cache!");
                exit(EXIT_FAILURE);
            }
        }
    }

    closedir(directory);
}

void * handle_cleanup(void * data) {

    cache_cleanup = (cache_t *) data;

    while(usleep(1000) == 0) {
        process_cleanup();
    }

    pthread_exit(NULL);
}