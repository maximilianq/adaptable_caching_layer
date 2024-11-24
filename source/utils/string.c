#include "string.h"

#include <string.h>

int equal_string(void * first, void * second) {
    return strcmp(first, second) == 0;
}

int hash_string(void * data) {

    char * pointer = data;
    int output = 0;

    char character;
    while ((character = *pointer++) != '\0') {
        output += character;
    }

    return output;
}