#define _GNU_SOURCE

#include "operations.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <pthread.h>

#include "calls.h"
#include "cache.h"
#include "lookahead.h"
#include "path.h"

bool initialized = false;

cache_t * cache = NULL;
pthread_t thread_lookahead;

void init() {

	// initialize cache
	cache = (cache_t *) malloc(sizeof(cache_t));
	cache_init(cache);

	pthread_create(&thread_lookahead, NULL, handle_lookahead, cache);

	initialized = true;
}

int acl_open(const char * path, int flags, mode_t mode) {

	if (!initialized)
		init();

	// expand path to full path
	char * source_path = malloc(PATH_MAX * sizeof(char));
	get_full_path(path, source_path, PATH_MAX);

	int source_file = sys_open(source_path, flags, mode);
	if (source_file == -1) {
		return source_file;
	}

	// check if file is in cache
	char * cache_path = cache_query(cache, source_path);
	int cache_file = -1;
	if (cache_path == NULL) {
		enqueue(&cache->c_lookahead, source_path);
	} else {
		cache_file = sys_open(cache_path, flags, mode);
	}

	// add data to mapping information
	pthread_mutex_lock(&cache->c_mutex_mapping[source_file]);
	cache->c_mapping[source_file] = malloc(sizeof(cache_mapping_t));
	cache->c_mapping[source_file]->cm_file = cache_file;
	strcpy(cache->c_mapping[source_file]->cm_path, source_path);
	cache->c_mapping[source_file]->cm_mode = mode;
	cache->c_mapping[source_file]->cm_upper = -1;
	cache->c_mapping[source_file]->cm_lower = -1;
	pthread_mutex_unlock(&cache->c_mutex_mapping[source_file]);

	struct flock lock;
	lock.l_type = (flags & O_RDONLY) == O_RDONLY ? F_RDLCK : F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	if (fcntl(source_file, F_SETLKW, &lock) == -1) {
		perror("ERROR: could not aquire lock on source file!");
		exit(EXIT_FAILURE);
	}

    return source_file;
}

ssize_t acl_read(int fd, void * buffer, size_t count) {

	if (!initialized)
		init();

	pthread_mutex_lock(&cache->c_mutex_mapping[fd]);

	if (cache->c_mapping[fd] == NULL) {
		pthread_mutex_unlock(&cache->c_mutex_mapping[fd]);
		return sys_read(fd, buffer, count);
	}

	if (cache->c_mapping[fd]->cm_file == -1) {
		pthread_mutex_unlock(&cache->c_mutex_mapping[fd]);
		char * source_path = malloc(PATH_MAX * sizeof(char));
		strcpy(source_path, cache->c_mapping[fd]->cm_path);
		enqueue(&cache->c_lookahead, source_path);
		return sys_read(fd, buffer, count);
	}

	// get current offset of filedescriptor
	off_t offset = lseek(fd, 0, SEEK_CUR);
	if (offset == -1) {
		perror("ERROR: could not get offset of filedescriptor!");
		exit(EXIT_FAILURE);
	}

	// set offset of original file descriptor to cached file descriptor
	if (lseek(cache->c_mapping[fd]->cm_file, offset, SEEK_SET) == -1) {
		perror("ERROR: could not set offset of cache file!");
		exit(EXIT_FAILURE);
	}

	// handle read using original read method
    ssize_t result = sys_read(cache->c_mapping[fd]->cm_file, buffer, count);

	// adapt offset of original filedescriptor
	if (lseek(fd, result, SEEK_CUR) == -1) {
		perror("ERROR: could not set offset of source file!");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&cache->c_mutex_mapping[fd]);

	return result;
}

ssize_t acl_write(int fd, const void * buffer, size_t count) {

	if (!initialized)
		init();

	pthread_mutex_lock(&cache->c_mutex_mapping[fd]);

	if (cache->c_mapping[fd] == NULL || cache->c_mapping[fd]->cm_file == -1) {
		pthread_mutex_unlock(&cache->c_mutex_mapping[fd]);
		return sys_write(fd, buffer, count);
	}

	ssize_t result = sys_write(cache->c_mapping[fd]->cm_file, buffer, count);

	pthread_mutex_unlock(&cache->c_mutex_mapping[fd]);

	return result;
}

int acl_close(int fd) {

	if (!initialized)
		init();

	pthread_mutex_lock(&cache->c_mutex_mapping[fd]);

	if (cache->c_mapping[fd] == NULL) {
		pthread_mutex_unlock(&cache->c_mutex_mapping[fd]);
		return sys_close(fd);
	}

	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	if (fcntl(fd, F_SETLKW, &lock) == -1) {
		perror("ERROR: could not release lock from source file!");
		exit(EXIT_FAILURE);
	}

	if (cache->c_mapping[fd]->cm_file != -1) {
		sys_close(cache->c_mapping[fd]->cm_file);
	}

	free(cache->c_mapping[fd]);
	cache->c_mapping[fd] = NULL;

	pthread_mutex_unlock(&cache->c_mutex_mapping[fd]);

    return sys_close(fd);
}