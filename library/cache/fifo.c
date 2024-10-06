#include "fifo.h"

#include <time.h>

unsigned long insert_fifo() {
    return time(NULL);
}

unsigned long update_fifo(unsigned long prev) {
    return prev;
}