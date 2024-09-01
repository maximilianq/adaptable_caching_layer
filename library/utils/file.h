#ifndef FILE_H
#define FILE_H

#include <stdlib.h>

void copy_file_interval(char * source, char * target, ssize_t start, ssize_t end);

void copy_data(int source_file, int cache_file);

void copy_file(const char * source_path, const char * cache_path);

#endif