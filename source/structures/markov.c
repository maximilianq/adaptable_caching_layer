#include "markov.h"

#include <stdlib.h>

void init_markov(markov_t * markov, int capacity) {
    init_internal_markov(&markov->e_internal, capacity);
    pthread_mutex_init(&markov->e_mutex, NULL);
}

void free_markov(markov_t * markov) {
    free_internal_markov(&markov->e_internal);
    pthread_mutex_destroy(&markov->e_mutex);
}

void resize_markov(markov_t * markov, int capacity) {
    pthread_mutex_lock(&markov->e_mutex);
    resize_internal_markov(&markov->e_internal, capacity);
    pthread_mutex_unlock(&markov->e_mutex);
}

void update_markov(markov_t * markov, char * key) {
    pthread_mutex_lock(&markov->e_mutex);
    update_internal_markov(&markov->e_internal, key);
    pthread_mutex_unlock(&markov->e_mutex);
}

char * retrieve_markov(markov_t * markov, char * key) {
    pthread_mutex_lock(&markov->e_mutex);
    char * result = retrieve_internal_markov(&markov->e_internal, key);
    pthread_mutex_unlock(&markov->e_mutex);
    return result;
}