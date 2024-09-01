#define _GNU_SOURCE

#include "intercept.h"

#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>

#include "acl.h"

int ladvise(const char * path, int flags) {
    return acl_advise(path, flags);
}

int open(const char * path, int flags, ...) {

    va_list args;
    va_start(args, flags);
    mode_t mode = flags & O_CREAT ? va_arg(args, mode_t) : 0;
    va_end(args);

    return acl_open(path, flags, mode);
}

int open64(const char * path, int flags, ...) {;
    va_list args;
    va_start(args, flags);
    mode_t mode = flags & O_CREAT ? va_arg(args, mode_t) : 0;
    va_end(args);

    return acl_open(path, flags, mode);
}

int close(int fd) {
    return acl_close(fd);
}

int read(int fd, void * buffer, size_t size) {
    return acl_read(fd, buffer, size);
}

int write(int fd, const void * buffer, size_t size) {
    return acl_write(fd, buffer, size);
}

int fsync(int fd) {
    return acl_sync(fd);
}

int posix_fadvise64(int fd, off_t offset, off_t len, int advice) {
    return 0;
}