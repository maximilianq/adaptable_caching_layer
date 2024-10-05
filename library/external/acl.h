#ifndef ACL_H
#define ACL_H

int acl_advise(const char * path, int flags, int priority);

void acl_prefetch(void * init, void * process, void * free);

void acl_select(void * prefetch, char * path);

#endif