#include "priority.h"

#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>

void listen_internal_priority(int index, unsigned long score, void * data, void * args) {
    int * current_index = retreive_internal_lookup(args, data);
    if (current_index != NULL)
        *current_index = index;
    else {
        printf("well...\n");/*
            void *array[10];
    size_t size;

    // Get the backtrace
    size = backtrace(array, 10);

    // Print the backtrace to stderr
    fprintf(stderr, "Error\n");
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);*/
    }
}

int compare_min_internal_priority(unsigned long first, unsigned long second) {
    return first - second;
}

int compare_max_internal_priority(unsigned long first, unsigned long second) {
    return second - first;
}

void init_internal_priority(internal_priority_t * priority, int capacity, hash_t hash, equal_t equal, int kind, insert_t insert, update_t update) {

    init_internal_lookup(&priority->p_lookup, capacity, hash, equal);

    if (kind == PRIO_MIN) {
        init_internal_heap(&priority->p_heap, capacity, compare_min_internal_priority, listen_internal_priority, &priority->p_lookup, insert, update);
    } else {
        init_internal_heap(&priority->p_heap, capacity, compare_max_internal_priority, listen_internal_priority, &priority->p_lookup, insert, update);
    }
}

void free_internal_priority(internal_priority_t * priority) {
    free_internal_lookup(&priority->p_lookup);
    free_internal_heap(&priority->p_heap);
}

void insert_internal_priority(internal_priority_t * priority, unsigned long score, void * data) {

    int * index = retreive_internal_lookup(&priority->p_lookup, data);
    if (index != NULL) {

        // update heap location with current score
        *index = update_internal_heap(&priority->p_heap, *index, score);

        // insert item together with index into lookup table
        insert_internal_lookup(&priority->p_lookup, data, index);

        return;
    }

    // allocate space for index in lookup table
    index = malloc(sizeof(int));

    // insert item together with index lookation into lookup table
    insert_internal_lookup(&priority->p_lookup, data, index);

    // insert into heap and store index in lookup
    *index = insert_internal_heap(&priority->p_heap, score, data);
}

void update_internal_priority(internal_priority_t * priority, unsigned long score, void * data) {

    // retrieve entry index from lookup table and return prematurely in case of miss
    int * index = retreive_internal_lookup(&priority->p_lookup, data);
    if (index == NULL)
        return;

    // update index with the resulting index of the entry in heap
    *index = update_internal_heap(&priority->p_heap, *index, score);
}

int remove_internal_priority(internal_priority_t * priority, void * data) {

    // remove item from lookup and return with -1 if item could not be found
    int * index = remove_internal_lookup(&priority->p_lookup, data);
    if (index == NULL) {
        return -1;
    }

    // remove item from heap and free index
    remove_internal_heap(&priority->p_heap, *index);
    free(index);

    return 0;
}

void * pop_internal_priority(internal_priority_t * priority) {

    // remove item from heap
    void * data = remove_internal_heap(&priority->p_heap, 0);
    if (data == NULL) {
        return NULL;
    }

    // remove entry from lookup and free index
    int * index = remove_internal_lookup(&priority->p_lookup, data);
    free(index);

    return data;
}
