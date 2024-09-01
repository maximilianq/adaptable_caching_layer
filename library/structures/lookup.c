#include "lookup.h"

void init_lookup(lookup_t * lookup, int capacity, hash_t hash, equal_t equal) {
    init_internal_lookup(&lookup->l_internal, capacity, hash, equal);
    pthread_mutex_init(&lookup->l_mutex, NULL);
}

void free_lookup(lookup_t * lookup) {
    free_internal_lookup(&lookup->l_internal);
    pthread_mutex_destroy(&lookup->l_mutex);
}

void insert_lookup(lookup_t * lookup, void * key, void * data) {

    pthread_mutex_lock(&lookup->l_mutex);

    insert_internal_lookup(&lookup->l_internal, key, data);

    pthread_mutex_unlock(&lookup->l_mutex);
}

void * retreive_lookup(lookup_t * lookup, void * key) {

    pthread_mutex_lock(&lookup->l_mutex);

    void * result = retreive_internal_lookup(&lookup->l_internal, key);

    pthread_mutex_unlock(&lookup->l_mutex);

    return result;
}

void * remove_lookup(lookup_t * lookup, void * key) {

    pthread_mutex_lock(&lookup->l_mutex);

    void * result = remove_internal_lookup(&lookup->l_internal, key);

    pthread_mutex_unlock(&lookup->l_mutex);

    return result;
}