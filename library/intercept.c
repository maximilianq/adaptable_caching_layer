#define _GNU_SOURCE

#include "intercept.h"

#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/limits.h>

#include "acl.h"
#include "utils/path.h"

int ladvise(const char * path, int flags) {
    return acl_advise(path, flags);
}

int open(const char * path, int flags, ...) {

    // expand relative path to full path
    char * full_path = malloc(PATH_MAX * sizeof(char));
    get_full_path(path, full_path, PATH_MAX);

    // decode the variadic arguments
    va_list args;
    va_start(args, flags);
    mode_t mode = flags & O_CREAT ? va_arg(args, mode_t) : 0;
    va_end(args);

    return acl_open(full_path, flags, mode);
}

int open64(const char * path, int flags, ...) {

    // expand relative path to full path
    char * full_path = malloc(PATH_MAX * sizeof(char));
    get_full_path(path, full_path, PATH_MAX);

    // decode the variadic arguments
    va_list args;
    va_start(args, flags);
    mode_t mode = flags & O_CREAT ? va_arg(args, mode_t) : 0;
    va_end(args);

    return acl_open(full_path, flags, mode);
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