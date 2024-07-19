#ifndef PATH_H
#define PATH_H

#include <stdlib.h>

char * get_source_path(const char * input, char * output, size_t size);

char * get_cache_path(const char * input, char * output, size_t size);

char * get_parent_path(const char * input, char * output, size_t size);

#endif