#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdlib.h>

int acl_open(const char * path, int flags, mode_t mode);

ssize_t acl_read(int fd, void * buffer, size_t count);

ssize_t acl_write(int fd, const void * buffer, size_t count);

int acl_close(int fd);

#endif