#include "extent.h"

#include <stdlib.h>

void init_extent(extent_t * extent) {
    init_internal_extent(&extent->e_internal);
    pthread_mutex_init(&extent->e_mutex, NULL);
}

void free_extent(extent_t * extent) {
    free_internal_extent(&extent->e_internal);
    pthread_mutex_destroy(&extent->e_mutex);
}

void push_extent(extent_t * extent, ssize_t lower, ssize_t upper) {
    pthread_mutex_lock(&extent->e_mutex);
    push_internal_extent(&extent->e_internal, lower, upper);
    pthread_mutex_unlock(&extent->e_mutex);
}

int pop_extent(extent_t * extent, ssize_t * lower, ssize_t * upper) {
    pthread_mutex_lock(&extent->e_mutex);
    int result = pop_internal_extent(&extent->e_internal, lower, upper);
    pthread_mutex_unlock(&extent->e_mutex);
    return result;
}