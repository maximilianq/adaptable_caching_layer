#include "priority.h"

void init_priority(priority_t * priority, int capacity, hash_t hash, equal_t equal, int kind) {
    init_internal_priority(&priority->p_internal, capacity, hash, equal, kind);
    pthread_mutex_init(&priority->p_mutex, NULL);
}

void free_priority(priority_t * priority) {
    free_internal_priority(&priority->p_internal);
    pthread_mutex_destroy(&priority->p_mutex);
}

void insert_priority(priority_t * priority, unsigned long score, void * data) {

    pthread_mutex_lock(&priority->p_mutex);

    insert_internal_priority(&priority->p_internal, score, data);

    pthread_mutex_unlock(&priority->p_mutex);
}

void update_priority(priority_t * priority, unsigned long score, void * data) {

    pthread_mutex_lock(&priority->p_mutex);

    update_internal_priority(&priority->p_internal, score, data);

    pthread_mutex_unlock(&priority->p_mutex);
}

void * remove_priority(priority_t * priority) {

    pthread_mutex_lock(&priority->p_mutex);

    void * result = remove_internal_priority(&priority->p_internal);

    pthread_mutex_unlock(&priority->p_mutex);

    return result;
}