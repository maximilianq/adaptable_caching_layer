#include "priority.h"

void init_priority(priority_t * priority, int capacity, hash_t hash, equal_t equal, int kind, insert_t insert, update_t update) {
    init_internal_priority(&priority->p_internal, capacity, hash, equal, kind, insert, update);
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

int remove_priority(priority_t * priority, void * data) {

    pthread_mutex_lock(&priority->p_mutex);

    int result = remove_internal_priority(&priority->p_internal, data);

    pthread_mutex_unlock(&priority->p_mutex);

    return result;
}

void * pop_priority(priority_t * priority) {

    pthread_mutex_lock(&priority->p_mutex);

    void * result = pop_internal_priority(&priority->p_internal);

    pthread_mutex_unlock(&priority->p_mutex);

    return result;
}