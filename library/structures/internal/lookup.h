#ifndef INTERNAL_LOOKUP_H
#define INTERNAL_LOOKUP_H

#include <stdlib.h>

// definition of signature of hash function
typedef int (* hash_t)(void * data);

// definition of signature of equal function
typedef int (* equal_t)(void * left, void * right);

// definition of structure lookup_entry
struct lookup_entry {

    // store the key of the lookup entry
    void * le_key;

    // store the data of the lookup entry
    void * le_data;

    // store reference to the following lookup entry in case of duplicates
    struct lookup_entry * le_entry;
};

// definition of type alias lookup_entry_t
typedef struct lookup_entry lookup_entry_t;

// definition of structure lookup
struct internal_lookup {

    // store array with references to the individual entries based on calculated hash
    lookup_entry_t ** l_entries;

    // keep track of size and capacity of lookup
    size_t l_size;
    size_t l_capacity;

    // hash and compare function of corresponding entry
    hash_t l_hash;
    equal_t l_equal;
};

// definition of type alias internal_lookup_t
typedef struct internal_lookup internal_lookup_t;

// definition of signature of init lookup function
void init_internal_lookup(internal_lookup_t * lookup, int capacity, hash_t hash, equal_t equal);

// definition of signature of free lookup function
void free_internal_lookup(internal_lookup_t * lookup);

// definition of signature of inserting into lookup function
void insert_internal_lookup(internal_lookup_t * lookup, void * key, void * data);

// definition of signature of retrieving from lookup function
void * retreive_internal_lookup(internal_lookup_t * lookup, void * key);

// definition of signature of removing from lookup function
void * remove_internal_lookup(internal_lookup_t * lookup, void * key);

#endif