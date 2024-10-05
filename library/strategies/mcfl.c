#include "mcfl.h"

#include <stdio.h>
#include <string.h>

#include "../cache.h"
#include "../state.h"

#include "../structures/queue.h"

void process_mcfl(state_t * state, char * source_path, markov_t * markov) {

    // retrieve most likely file following the current one
    char * candidate_path = retrieve_markov(markov, source_path);

    // insert file into low priority queue
    if (candidate_path != NULL) {
        enqueue(&state->m_queue_low, candidate_path);
    }

    // update markov chain with current access
    update_markov(markov, source_path);
}

void * handle_mcfl(void * data) {

    state_t * state = data;

    markov_t markov;
    init_markov(&markov, 64);

    char * value;
    while(1) {

        if ((value = dequeue(&state->m_queue_high)) != NULL || (value = dequeue(&state->m_queue_low)) != NULL) {
            insert_cache(&state->m_cache, value);
            free(value);
            continue;
        }

        value = dequeue(&state->m_queue_prefetch);
        if (value != NULL) {
            process_mcfl(state, value, &markov);
            free(value);
        }
    }
}