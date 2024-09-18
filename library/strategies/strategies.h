#ifndef STRATEGIES_H

    #define STRATEGIES_H

    #include "../constants.h"

    #if defined(FETCH_STRATEGY_FSDL)
        #include "fsdl.h"
        #define handle_lookahead handle_fsdl
    #elif defined(FETCH_STRATEGY_FRDL)
        #include "frdl.h"
        #define handle_lookahead handle_frdl
    #elif defined(FETCH_STRATEGY_PSDL)
        #include "psdl.h"
        #define handle_lookahead handle_psdl
    #elif defined(FETCH_STRATEGY_PRDL)
        #include "prdl.h"
        #define handle_lookahead handle_prdl
    #elif defined(FETCH_STRATEGY_MCFL)
        #include "mcfl.h"
        #define handle_lookahead handle_mcfl
    #endif

#endif //STRATEGIES_H
