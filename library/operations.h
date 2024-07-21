#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdlib.h>
#include <limits.h>

#include "queue.h"

typedef struct {
    int c_file;
    char c_path[PATH_MAX];
    mode_t c_flags;
    size_t c_lower;
    size_t c_upper;
} cache_item_t;

typedef struct {
    char c_path[PATH_MAX];
    size_t c_size;
    time_t c_time;
} cache_element_t;

typedef struct {
    queue_t c_lookahead;
    queue_t c_lru;
    cache_item_t * c_items[4096];
    pthread_mutex_t c_items_mutex[4096];
} cache_t;

int acl_open(const char * path, int flags, mode_t mode);

ssize_t acl_read(int fd, void * buffer, size_t count);

ssize_t acl_write(int fd, const void * buffer, size_t count);

int acl_close(int fd);

#endif