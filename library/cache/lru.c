#include "lru.h"

#include <time.h>

unsigned long insert_lru() {
    return time(NULL);
}

unsigned long update_lru(unsigned long prev) {
    return time(NULL);
}