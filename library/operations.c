#define _GNU_SOURCE

#include "operations.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <pthread.h>

#include "path.h"
#include "queue.h"
#include "file.h"
#include "calls.h"
#include "lookahead.h"
#include "cleanup.h"

pthread_t thread_lookahead;
pthread_t thread_cleanup;

bool initialized = false;

cache_t * cache = NULL;

void init() {

	cache = (cache_t *) malloc(sizeof(cache_t));

	init_queue(&cache->c_lookahead, 4096);

	for (int i = 0; i < 4096; i++) {
		pthread_mutex_init(&cache->c_items_mutex[i], NULL);
	}

	pthread_create(&thread_lookahead, NULL, handle_lookahead, cache);
	pthread_create(&thread_lookahead, NULL, handle_cleanup, cache);

	initialized = true;
}

int acl_open(const char * path, int flags, mode_t mode) {

	if (!initialized)
		init();

	// get full path and cache path from relative path
	char source_path[PATH_MAX];
	char cache_path[PATH_MAX];

    get_source_path(path, source_path, PATH_MAX);
    if (get_cache_path(source_path, cache_path, PATH_MAX) == NULL) {
        return sys_open(path, flags, 0);
    }

    char * process_path = malloc(PATH_MAX * sizeof(char));
    strcpy(process_path, source_path);

    int source_file, cache_file;
	if ((source_file = sys_open(source_path, flags, mode)) == -1) {
		return source_file;
	}

	pthread_mutex_lock(&cache->c_items_mutex[source_file]);

    cache->c_items[source_file] = (cache_item_t *) malloc(sizeof(cache_item_t));
	cache->c_items[source_file]->c_file = -1;
	strcpy(cache->c_items[source_file]->c_path, cache_path);
	cache->c_items[source_file]->c_lower = -1;
	cache->c_items[source_file]->c_upper = -1;
	cache->c_items[source_file]->c_flags = flags;

	if ((cache_file = sys_open(cache_path, flags, mode)) == -1) {

        if (errno != ENOENT) {
		    perror("ERROR: could not open cache file.");
		    exit(EXIT_FAILURE);
        }

	    enqueue(&(cache->c_lookahead), process_path);

		pthread_mutex_unlock(&cache->c_items_mutex[source_file]);

        return source_file;
	}

    time_t mtime;
    if (fgetxattr(cache_file, "user.mtime", &mtime, sizeof(mtime)) == -1) {

        if (errno != ENODATA) {
	        perror("ERROR: could not get xattr from cache file.");
	        exit(EXIT_FAILURE);
        }

        sys_close(cache_file);

    	pthread_mutex_unlock(&cache->c_items_mutex[source_file]);

        return source_file;
    }

    struct stat source_stat;
    if (fstat(source_file, &source_stat) == -1) {
	    perror("ERROR: could not stat source file.");
	    exit(EXIT_FAILURE);
    }

    // invalidate cache if modified after caching
    if (mtime != source_stat.st_mtime) {
        sys_close(cache_file);

    	pthread_mutex_unlock(&cache->c_items_mutex[source_file]);

        return source_file;
    }

    // aquire corresponding locks on source file to prevent cache inconistencies
    struct flock lock;
    lock.l_type = (flags & O_RDONLY) == O_RDONLY ? F_RDLCK : F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(source_file, F_SETLKW, &lock) == -1) {
        perror("ERROR: could not aquire lock on source file!");
        exit(EXIT_FAILURE);
    }

    cache->c_items[source_file]->c_file = cache_file;

	pthread_mutex_unlock(&cache->c_items_mutex[source_file]);

    return source_file;
}

ssize_t acl_read(int fd, void * buffer, size_t count) {

	if (!initialized)
		init();

	pthread_mutex_lock(&cache->c_items_mutex[fd]);

    if (cache->c_items[fd] == NULL || cache->c_items[fd]->c_file == -1) {
    	pthread_mutex_unlock(&cache->c_items_mutex[fd]);
        return sys_read(fd, buffer, count);
    }

    // set offset according to offset of source file
    off_t offset = lseek(fd, 0, SEEK_CUR);
    lseek(cache->c_items[fd]->c_file, offset, SEEK_SET);

    ssize_t bytes_read = sys_read(cache->c_items[fd]->c_file, buffer, count);

    // set new offset according to bytes read
    lseek(fd, bytes_read, SEEK_CUR);

	pthread_mutex_unlock(&cache->c_items_mutex[fd]);

    return bytes_read;
}

ssize_t acl_write(int fd, const void * buffer, size_t count) {

	if (!initialized)
		init();

	pthread_mutex_lock(&cache->c_items_mutex[fd]);

	if (cache->c_items[fd] == NULL || cache->c_items[fd]->c_file == -1) {
		pthread_mutex_unlock(&cache->c_items_mutex[fd]);
		return sys_write(fd, buffer, count);
    }

    // set offset according to offset of source file
    off_t offset = lseek(fd, 0, SEEK_CUR);
    lseek(cache->c_items[fd]->c_file, offset, SEEK_SET);

	ssize_t bytes_write = sys_write(cache->c_items[fd]->c_file, buffer, count);

	// calculate modification bound of current request
	ssize_t lower = offset;
	ssize_t upper = offset + bytes_write;

	// set total modification bounds of file as minimum of lower and maximum of upper
    cache->c_items[fd]->c_lower = cache->c_items[fd]->c_lower == -1 ? lower : (cache->c_items[fd]->c_lower < lower ? cache->c_items[fd]->c_lower : lower);
    cache->c_items[fd]->c_upper = cache->c_items[fd]->c_upper == -1 ? upper : (cache->c_items[fd]->c_upper > upper ? cache->c_items[fd]->c_upper : upper);

    // set new offset according to bytes read
    lseek(fd, bytes_write, SEEK_CUR);

	pthread_mutex_unlock(&cache->c_items_mutex[fd]);

	return bytes_write;
}

int acl_close(int fd) {

	if (!initialized)
		init();

	pthread_mutex_lock(&cache->c_items_mutex[fd]);

    // if file not managed close only source file
	if (cache->c_items[fd] == NULL) {
		pthread_mutex_unlock(&cache->c_items_mutex[fd]);
        return sys_close(fd);
    }

    // if file is not in cache close only source file
    if (cache->c_items[fd]->c_file == -1) {
        free(cache->c_items[fd]);
        cache->c_items[fd] = NULL;
    	pthread_mutex_unlock(&cache->c_items_mutex[fd]);
        return sys_close(fd);
    }

    // if file in cache has been modified write changes to disk
    if (cache->c_items[fd]->c_lower != -1 && cache->c_items[fd]->c_upper != -1) {
        copy_data_interval(cache->c_items[fd]->c_file, fd, cache->c_items[fd]->c_lower, (cache->c_items[fd]->c_upper - cache->c_items[fd]->c_lower));
    }

    // close cache file
	sys_close(cache->c_items[fd]->c_file);

    // clear cache struct on file close
    free(cache->c_items[fd]);
    cache->c_items[fd] = NULL;

	pthread_mutex_unlock(&cache->c_items_mutex[fd]);

    // release file lock
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("ERROR: could not release lock on source file!");
        exit(EXIT_FAILURE);
    }

    // close source file
    return sys_close(fd);
}