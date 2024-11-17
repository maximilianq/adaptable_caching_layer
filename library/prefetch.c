#include "prefetch.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

void prefetch_init(prefetch_t * prefetch, cache_t * cache, prefetch_strategy_t strategy) {

    init_queue(&prefetch->p_history, 64768);

    init_queue(&prefetch->p_high, 1024);
    init_queue(&prefetch->p_low, 64768);

    prefetch->p_strategy = strategy;
    prefetch->p_status = 1;

    // format arguments to be handed over to the
    arguments_t * arguments = malloc(sizeof(arguments_t));
    arguments->a_cache = cache;
    arguments->a_prefetch = prefetch;

    pthread_create(&prefetch->p_thread, NULL, prefetch_handler, arguments);
}

void prefetch_free(prefetch_t * prefetch) {

    free_queue(&prefetch->p_history);

    free_queue(&prefetch->p_high);
    free_queue(&prefetch->p_low);
}

void prefetch_replace(prefetch_t * prefetch, cache_t * cache, prefetch_strategy_t strategy) {

    // stop current prefetching thread
    prefetch->p_status = 0;
    pthread_join(prefetch->p_thread, NULL);

    // replace strategy and set status to 1 again
    prefetch->p_strategy = strategy;
    prefetch->p_status = 1;

    // format arguments to be handed over to the
    arguments_t * arguments = malloc(sizeof(arguments_t));
    arguments->a_cache = cache;
    arguments->a_prefetch = prefetch;

    pthread_create(&prefetch->p_thread, NULL, prefetch_handler, arguments);
}

void prefetch_predict(prefetch_t * prefetch, char * path) {

    char * insert_path = malloc(PATH_MAX * sizeof(char));
    strcpy(insert_path, path);

    push_queue(&prefetch->p_low, insert_path);
}

void prefetch_inform(prefetch_t * prefetch, char * path) {

    char * insert_path = malloc(PATH_MAX * sizeof(char));
    strcpy(insert_path, path);

    push_queue(&prefetch->p_high, insert_path);
}

void * prefetch_handler(void * data) {
   
    // retrieve arguments from struct
    prefetch_t * prefetch = ((arguments_t *) data)->a_prefetch;
    cache_t * cache = ((arguments_t *) data)->a_cache;

    // free memory of argument structure
    free(data);

    if (prefetch->p_strategy.ps_init != NULL)
        prefetch->p_strategy.ps_init(prefetch);

    char * path;
    while(prefetch->p_status) {

        if ((path = pop_queue(&prefetch->p_high)) != NULL || (path = pop_queue(&prefetch->p_low)) != NULL) {
            insert_cache(cache, path);
            free(path);
            continue;
        }

        path = pop_queue(&prefetch->p_history);
        if (path != NULL) {
            if (prefetch->p_strategy.ps_process != NULL)
                prefetch->p_strategy.ps_process(prefetch, path);
            free(path);
        }

	    usleep(100);
    }

    if (prefetch->p_strategy.ps_free != NULL)
        prefetch->p_strategy.ps_free(prefetch);

    pthread_exit(NULL);
}