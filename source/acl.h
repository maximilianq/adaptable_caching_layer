#ifndef ACL_H
#define ACL_H

#include <stdlib.h>

#include "prefetch.h"

#define ACL_WILLNEED 1
#define ACL_DONTNEED 2

void acl_init();

int acl_advise(const char * path, int flags);

void acl_prefetch(prefetch_strategy_t strategy);

void acl_cache(cache_policy_t policy);

void acl_select(prefetch_t * prefetch, char * path);

int acl_open(const char * path, int flags, mode_t mode);

ssize_t acl_read(int fd, void * buffer, size_t count);

ssize_t acl_write(int fd, const void * buffer, size_t count);

int acl_close(int fd);

int acl_sync(int fd);

#endif