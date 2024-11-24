#ifndef VARIANT_H
#define VARIANT_H

#include "abfp.h"
prefetch_strategy_t abfp_strategy = {
    .ps_init = NULL,
    .ps_free = NULL,
    .ps_process = process_abfp
};

#include "fsdp.h"
prefetch_strategy_t fsdp_strategy = {
    .ps_init = NULL,
    .ps_free = NULL,
    .ps_process = process_fsdp
};

#include "mcfp.h"
prefetch_strategy_t mcfp_strategy = {
    .ps_init = init_mcfp,
    .ps_free = free_mcfp,
    .ps_process = process_fsdp
};

#endif