#ifndef EXTENT_H
#define EXTENT_H

#include <pthread.h>
#include <semaphore.h>

#include "internal/extent.h"

struct extent {
    internal_extent_t e_internal;
    pthread_mutex_t e_mutex;
};

typedef struct extent extent_t;

void init_extent(extent_t * extent);

void free_extent(extent_t * extent);

void push_extent(extent_t * extent, ssize_t lower, ssize_t upper);

int pop_extent(extent_t * extent, ssize_t * lower, ssize_t * upper);

#endif