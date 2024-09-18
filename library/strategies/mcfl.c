#include "mcfl.h"

#include <stdio.h>
#include <string.h>

#include "../cache.h"
#include "../memory.h"

#include "../structures/queue.h"

void process_mcfl(memory_t * memory, char * source_path, markov_t * markov) {

    // retrieve most likely file following the current one
    char * candidate_path = retrieve_markov(markov, source_path);

    // insert file into low priority queue
    if (candidate_path != NULL) {
        enqueue(&memory->m_queue_low, candidate_path);
    }

    // update markov chain with current access
    update_markov(markov, source_path);
}

void * handle_mcfl(void * data) {

    memory_t * memory = data;

    markov_t markov;
    init_markov(&markov, 64);

    char * value;
    while(1) {

        if ((value = dequeue(&memory->m_queue_high)) != NULL || (value = dequeue(&memory->m_queue_low)) != NULL) {
            insert_cache(&memory->m_cache, value);
            free(value);
            continue;
        }

        value = dequeue(&memory->m_queue_prefetch);
        if (value != NULL) {
            process_mcfl(memory, value, &markov);
            free(value);
        }
    }
}