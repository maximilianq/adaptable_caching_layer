#ifndef FSDL_H
#define FSDL_H

#include "../prefetch.h"

void init_fsdl(prefetch_t * prefetch);

void free_fsdl(prefetch_t * prefetch);

void process_fsdl(prefetch_t * prefetch, char * source_path);

#endif