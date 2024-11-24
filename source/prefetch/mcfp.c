#include "mcfp.h"

#include <stdio.h>

#include "../prefetch.h"
#include "../structures/markov.h"

void init_mcfp(prefetch_t * prefetch) {
    prefetch->p_data = malloc(sizeof(markov_t));
    init_markov(prefetch->p_data, 128);
}

void free_mcfp(prefetch_t * prefetch) {
    free_markov(prefetch->p_data);
}

void process_mcfp(prefetch_t * prefetch, char * path) {

    // retrieve most likely file following the current one
    char * candidate_path = retrieve_markov(prefetch->p_data, path);

    // insert file into low priority queue
    if (candidate_path != NULL) {
        prefetch_predict(prefetch, candidate_path);
    } else {
	    prefetch_predict(prefetch, path);
    }

    // update markov chain with current access
    update_markov(prefetch->p_data, path);
}
