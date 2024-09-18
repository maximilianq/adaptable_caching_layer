#ifndef MCFL_H
#define MCFL_H

#include "../memory.h"
#include "../structures/markov.h"

void process_mcfl(memory_t * memory, char * path, markov_t * markov);

void * handle_mcfl(void * data);

#endif //MCFL_H
