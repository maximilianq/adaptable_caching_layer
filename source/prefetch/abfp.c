#include "abfp.h"

void process_abfp(prefetch_t * prefetch, cache_miss_t cache_miss) {
    prefetch_predict(prefetch, cache_miss.cm_path);
}