#ifndef CALLS_H
#define CALLS_H

#include <stdlib.h>
#include <unistd.h>

int sys_open(const char * path, int flags, mode_t mode);

int sys_close(int fd);

int sys_read(int fd, void * buffer, size_t size);

int sys_write(int fd, const void * buffer, size_t size);

int sys_sync(int fd);

pid_t sys_fork();

#endif
