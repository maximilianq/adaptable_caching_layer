#ifndef mcfp_H
#define mcfp_H

#include "../prefetch.h"

void init_mcfp(prefetch_t * prefetch);

void free_mcfp(prefetch_t * prefetch);

void process_mcfp(prefetch_t * prefetch, char * path);

#endif