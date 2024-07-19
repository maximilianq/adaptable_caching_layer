#define _GNU_SOURCE

#include "calls.h"

#include <dlfcn.h>
#include <stdlib.h>

int (* __open)(const char *, int, ...) = NULL;
int (* __close)(int) = NULL;
int (* __read)(int, void *, size_t) = NULL;
int (* __write)(int, const void *, size_t) = NULL;

int sys_open(const char * path, int flags, mode_t mode) {
    if (__open == NULL) __open = dlsym(RTLD_NEXT, "open");
    return __open(path, flags, mode);
}

int sys_close(int fd) {
    if (__close == NULL) __close = dlsym(RTLD_NEXT, "close");
    return __close(fd);
}

int sys_read(int fd, void * buffer, size_t size) {
    if (__read == NULL) __read = dlsym(RTLD_NEXT, "read");
    return __read(fd, buffer, size);
}

int sys_write(int fd, const void * buffer, size_t size) {
    if (__write == NULL) __write = dlsym(RTLD_NEXT, "write");
    return __write(fd, buffer, size);
}