#ifndef MARKOV_H
#define MARKOV_H

#include <pthread.h>
#include <semaphore.h>

#include "internal/markov.h"

struct markov {
    internal_markov_t e_internal;
    pthread_mutex_t e_mutex;
};

typedef struct markov markov_t;

void init_markov(markov_t * markov, int capacity);

void free_markov(markov_t * markov);

void resize_markov(markov_t * markov, int capacity);

void update_markov(markov_t * markov, char * path);

char * retrieve_markov(markov_t * markov, char * path);

#endif
