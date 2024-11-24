#ifndef LOOKUP_H
#define LOOKUP_H

#include <pthread.h>

#include "internal/lookup.h"

struct lookup {
    internal_lookup_t l_internal;
    pthread_mutex_t l_mutex;
};

// definition of type alias lookup_t
typedef struct lookup lookup_t;

// definition of signature of init lookup function
void init_lookup(lookup_t * lookup, int capacity, hash_t hash, equal_t equal);

// definition of signature of free lookup function
void free_lookup(lookup_t * lookup);

// definition of signature of inserting into lookup function
void insert_lookup(lookup_t * lookup, void * key, void * data);

// definition of signature of retrieving from lookup function
void * retreive_lookup(lookup_t * lookup, void * key);

// definition of signature of removing from lookup function
void * remove_lookup(lookup_t * lookup, void * key);

#endif