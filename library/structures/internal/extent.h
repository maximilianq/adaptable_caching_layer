#ifndef INTERNAL_EXTENT_H
#define INTERNAL_EXTENT_H

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#include <stdlib.h>

struct extent_entry {
    ssize_t ee_lower;
    ssize_t ee_upper;
    struct extent_entry * ee_next;
};

typedef struct extent_entry extent_entry_t;

struct internal_extent {
    extent_entry_t * e_root;
    int e_size;
};

typedef struct internal_extent internal_extent_t;

void init_internal_extent(internal_extent_t * extent);

void free_internal_extent(internal_extent_t * extent);

void push_internal_extent(internal_extent_t * extent, ssize_t lower, ssize_t upper);

int pop_internal_extent(internal_extent_t * extent, ssize_t * lower, ssize_t * upper);

#endif