#include "prefetch.h"

#include <stdio.h>
#include <string.h>

void prefetch_init(prefetch_t * prefetch, init_t init, process_t process, free_t free) {

    init_queue(&prefetch->p_history, 1024);

    init_queue(&prefetch->p_high, 1024);
    init_queue(&prefetch->p_low, 1024);

    prefetch->p_init = init;
    prefetch->p_process = process;
    prefetch->p_free = free;

    prefetch->p_status = 1;
}

void prefetch_free(prefetch_t * prefetch) {

    free_queue(&prefetch->p_history);

    free_queue(&prefetch->p_high);
    free_queue(&prefetch->p_low);
}

void prefetch_replace(prefetch_t * prefetch, cache_t * cache, init_t init, process_t process, free_t free) {

    // stop current prefetching thread
    prefetch->p_status = 0;
    pthread_join(prefetch->p_thread, NULL);

    prefetch->p_init = init;
    prefetch->p_process = process;
    prefetch->p_free = free;

    arguments_t * arguments = malloc(sizeof(arguments_t));
    arguments->a_cache = cache;
    arguments->a_prefetch = prefetch;

    prefetch->p_status = 1;

    pthread_create(&prefetch->p_thread, NULL, prefetch_handler, arguments);
}

void prefetch_predict(prefetch_t * prefetch, char * path) {

    char * insert_path = malloc(PATH_MAX * sizeof(char));
    strcpy(insert_path, path);

    enqueue(&prefetch->p_low, insert_path);
}

void * prefetch_handler(void * data) {

    // retrieve arguments from struct
    prefetch_t * prefetch = ((arguments_t *) data)->a_prefetch;
    cache_t * cache = ((arguments_t *) data)->a_cache;

    // free memory of argument structure
    free(data);

    if (prefetch->p_init != NULL)
        prefetch->p_init(prefetch);

    char * path;
    while(prefetch->p_status) {

        if ((path = dequeue(&prefetch->p_high)) != NULL || (path = dequeue(&prefetch->p_low)) != NULL) {
            insert_cache(cache, path);
            free(path);
            continue;
        }

        path = dequeue(&prefetch->p_history);
        if (path != NULL) {
            prefetch->p_process(prefetch, path);
            free(path);
        }
    }

    if (prefetch->p_free != NULL)
        prefetch->p_free(prefetch);

    pthread_exit(NULL);
}