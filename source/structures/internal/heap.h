#ifndef INTERNAL_HEAP_H
#define INTERNAL_HEAP_H

typedef int (* compare_t)(unsigned long first, unsigned long second);
typedef void (* listen_t)(int index, unsigned long score, void * data, void * args);

typedef unsigned long (* insert_t)();
typedef unsigned long (* update_t)(unsigned long prev);

struct heap_entry {
    unsigned long he_score;
    void * he_data;
};

typedef struct heap_entry heap_entry_t;

struct internal_heap {

    heap_entry_t ** h_entries;

    int h_size, h_capacity;

    compare_t h_compare;

    listen_t h_listen;
    void * h_listen_args;

    insert_t h_insert;
    update_t h_update;
};

typedef struct internal_heap internal_heap_t;

void init_internal_heap(internal_heap_t * heap, int capacity, compare_t compare, listen_t listen, void * listen_data, insert_t insert, update_t update);

void free_internal_heap(internal_heap_t * heap);

int insert_internal_heap(internal_heap_t * heap, unsigned long score, void * data);

void * remove_internal_heap(internal_heap_t * heap, int index);

int update_internal_heap(internal_heap_t * heap, int index, unsigned long score);

#endif
