#ifndef ACL_H
#define ACL_H

#include <stdlib.h>

void acl_init();

int acl_advise(const char * path, int flags);

void acl_prefetch(void * init, void * process, void * free);

void acl_select(void * prefetch, char * path);

int acl_open(const char * path, int flags, mode_t mode);

ssize_t acl_read(int fd, void * buffer, size_t count);

ssize_t acl_write(int fd, const void * buffer, size_t count);

int acl_close(int fd);

int acl_sync(int fd);

#endif