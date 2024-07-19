#ifndef FILE_H
#define FILE_H

#include <stdlib.h>

void copy_data_interval(int source, int target, off_t offset, size_t size);

void copy_data(int source_file, int cache_file);

void copy_file(const char * source_path, const char * cache_path);

#endif