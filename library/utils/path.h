#ifndef PATH_H
#define PATH_H

#include <stdlib.h>

char * get_full_path(const char * input, char * output, size_t size);

char * get_cache_path(const char * input, char * output, size_t size);

int is_cache_path(const char * input);

char * get_parent_path(const char * input, char * output);

int compare_path(const void * a, const void * b);

#endif