#include "lookup.h"

#include <stdlib.h>

void init_internal_lookup(internal_lookup_t * lookup, int capacity, hash_t hash, equal_t equal) {

    lookup->l_entries = malloc(capacity * sizeof(lookup_entry_t *));
    for (int i = 0; i < capacity; i++) lookup->l_entries[i] = NULL;

    lookup->l_size = 0;
    lookup->l_capacity = capacity;

    lookup->l_hash = hash;
    lookup->l_equal = equal;
}

void free_internal_lookup(internal_lookup_t * lookup) {
    free(lookup->l_entries);
}

void insert_internal_lookup(internal_lookup_t * lookup, void * key, void * data) {

    int hash = lookup->l_hash(key) % lookup->l_capacity;

    lookup_entry_t * entry = lookup->l_entries[hash];
    lookup_entry_t * previous = NULL;

    while (entry != NULL) {

    	if (lookup->l_equal(entry->le_key, key) == 1)
    	    return;

    	previous = entry;
    	entry = entry->le_entry;
    }

    entry = malloc(sizeof(lookup_entry_t));
    entry->le_key = key;
    entry->le_data = data;
    entry->le_entry = NULL;

    if (previous == NULL) {
        lookup->l_entries[hash] = entry;
    } else {
        previous->le_entry = entry;
    }

    lookup->l_size = lookup->l_size + 1;
}

void * retreive_internal_lookup(internal_lookup_t * lookup, void * key) {

    int hash = lookup->l_hash(key) % lookup->l_capacity;

    lookup_entry_t * entry = lookup->l_entries[hash];

    while (entry != NULL) {

    	if (lookup->l_equal(entry->le_key, key) == 1) {
    	    return entry->le_data;
    	}

    	entry = entry->le_entry;
    }

    return NULL;
}

void * remove_internal_lookup(internal_lookup_t * lookup, void * key) {

    int hash = lookup->l_hash(key) % lookup->l_capacity;

    lookup_entry_t * entry = lookup->l_entries[hash];
    lookup_entry_t * previous = NULL;

    while (entry != NULL) {

    	if (lookup->l_equal(entry->le_key, key) == 1) {

            if (previous == NULL) {
                lookup->l_entries[hash] = entry->le_entry;
            } else {
                previous->le_entry = entry->le_entry;
            }

    	    void * output = entry->le_data;

    	    free(entry);

    	    lookup->l_size = lookup->l_size - 1;

            return output;
    	}

    	previous = entry;
    	entry = entry->le_entry;
    }

    return NULL;
}