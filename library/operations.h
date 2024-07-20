#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdlib.h>
#include <limits.h>

#include "queue.h"

typedef struct {
    int c_cache;
    char c_path[PATH_MAX];
    size_t c_lower;
    size_t c_upper;
} cache_item_t;

typedef struct {
    queue_t c_queue;
    cache_item_t * c_items[4096];
    pthread_mutex_t c_mutex;
} cache_t;

int acl_open(const char * path, int flags, mode_t mode);

ssize_t acl_read(int fd, void * buffer, size_t count);

ssize_t acl_write(int fd, const void * buffer, size_t count);

int acl_close(int fd);

#endif