#ifndef PATH_H
#define PATH_H

char * get_full_path(const char * input, char * output);

char * get_cache_path(const char * input, char * output);

int is_cacheable_path(const char * input);

char * get_parent_path(const char * input, char * output);

int compare_path(const void * a, const void * b);

#endif