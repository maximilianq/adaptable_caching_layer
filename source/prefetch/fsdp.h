#ifndef FSDP_H
#define FSDP_H

#include "../prefetch.h"

void process_fsdp(prefetch_t * prefetch, cache_miss_t cache_miss);

#endif