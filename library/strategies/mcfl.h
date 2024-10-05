#ifndef MCFL_H
#define MCFL_H

#include "../state.h"
#include "../structures/markov.h"

void process_mcfl(state_t * state, char * path, markov_t * markov);

void * handle_mcfl(void * data);

#endif //MCFL_H
