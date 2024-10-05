#ifndef MCFL_H
#define MCFL_H

#include "../prefetch.h"

void init_mcfl(prefetch_t * prefetch);

void free_mcfl(prefetch_t * prefetch);

void process_mcfl(prefetch_t * prefetch, char * path);

#endif //MCFL_H
