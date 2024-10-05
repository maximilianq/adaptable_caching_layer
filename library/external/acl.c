#include "acl.h"

#include <stdbool.h>
#include <stdio.h>

// variable for storing if warning text has already been printed to avoid print it multiple times
bool warned = false;

// dummy implementation of the hinting interface enabling applications to hint their future use of files
int ACL_WILLNEED = 1;
int ACL_DONTNEED = 2;

int acl_advise(const char * path, int flags, int priority) {

    if (!warned) {
        printf("WARNING: acl library was not loaded even though acl method is implemented!\n");
        warned = true;
    }

    return 0;
}

// dummy implementation of the customization interface enabling application specific prefetching logic
void acl_prefetch(void * init, void * process, void * free) {

    if (!warned) {
        printf("WARNING: acl library was not loaded even though acl method is implemented!\n");
        warned = true;
    }
}

void acl_select(void * prefetch, char * path) {

    if (!warned) {
        printf("WARNING: acl library was not loaded even though acl method is implemented!\n");
        warned = true;
    }
}