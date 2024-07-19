#define _GNU_SOURCE

#include "operations.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
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
queue_t queue_lookahead;
bool running_lookahead = false;

pthread_t thread_cleanup;
bool running_cleanup = false;

typedef struct {
	int c_cache;
	size_t c_lower;
	size_t c_upper;
} cache_item_t;

cache_item_t * cache[1024];

int acl_open(const char * path, int flags, mode_t mode) {

	// get full path and cache path from relative path
	char source_path[PATH_MAX];
	char cache_path[PATH_MAX];

    get_source_path(path, source_path, PATH_MAX);
    if (get_cache_path(source_path, cache_path, PATH_MAX) == NULL) {
        return sys_open(path, flags, 0);
    }

    char * process_path = malloc(PATH_MAX * sizeof(char));
    strcpy(process_path, source_path);

	// launch helper thread that caches files in directory
    if (!running_lookahead) {
        init_queue(&queue_lookahead);
		running_lookahead = true;
        pthread_create(&thread_lookahead, NULL, handle_lookahead, &queue_lookahead);
	}

	// launch helper thread that caches files in directory
    //if (!running_cleanup) {
    //    pthread_create(&thread_lookahead, NULL, handle_cleanup, NULL);
	//	running_cleanup = true;
	//}

    int source_file, cache_file;
	if ((source_file = sys_open(source_path, flags, mode)) == -1) {
		return source_file;
	}

    cache[source_file] = (cache_item_t *) malloc(sizeof(cache_item_t));
	cache[source_file]->c_cache = -1;
	cache[source_file]->c_lower = -1;
	cache[source_file]->c_upper = -1;

	if ((cache_file = sys_open(cache_path, flags, mode)) == -1) {

        if (errno != ENOENT) {
		    perror("ERROR: could not open cache file.");
		    exit(EXIT_FAILURE);
        }

	    enqueue(&queue_lookahead, process_path);

        return source_file;
	}

    time_t mtime;
    if (fgetxattr(cache_file, "user.mtime", &mtime, sizeof(mtime)) == -1) {

        if (errno != ENODATA) {
	        perror("ERROR: could not get xattr from cache file.");
	        exit(EXIT_FAILURE);
        }

        sys_close(cache_file);

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

    cache[source_file]->c_cache = cache_file;

    return source_file;
}

ssize_t acl_read(int fd, void * buffer, size_t count) {

    if (cache[fd] == NULL || cache[fd]->c_cache == -1) {
        return sys_read(fd, buffer, count);
    }

    // set offset according to offset of source file
    off_t offset = lseek(fd, 0, SEEK_CUR);
    lseek(cache[fd]->c_cache, offset, SEEK_SET);

    ssize_t bytes_read = sys_read(cache[fd]->c_cache, buffer, count);

    // set new offset according to bytes read
    lseek(fd, bytes_read, SEEK_CUR);

    return bytes_read;
}

ssize_t acl_write(int fd, const void * buffer, size_t count) {

	if (cache[fd] == NULL || cache[fd]->c_cache == -1) {
		return sys_write(fd, buffer, count);
    }

    // set offset according to offset of source file
    off_t offset = lseek(fd, 0, SEEK_CUR);
    lseek(cache[fd]->c_cache, offset, SEEK_SET);

	ssize_t bytes_write = sys_write(cache[fd]->c_cache, buffer, count);

	// calculate modification bound of current request
	ssize_t lower = offset;
	ssize_t upper = offset + bytes_write;

	// set total modification bounds of file as minimum of lower and maximum of upper
    cache[fd]->c_lower = cache[fd]->c_lower == -1 ? lower : (cache[fd]->c_lower < lower ? cache[fd]->c_lower : lower);
    cache[fd]->c_upper = cache[fd]->c_upper == -1 ? upper : (cache[fd]->c_upper > upper ? cache[fd]->c_upper : upper);

    // set new offset according to bytes read
    lseek(fd, bytes_write, SEEK_CUR);

	return bytes_write;
}

int acl_close(int fd) {

    // if file not managed close only source file
	if (cache[fd] == NULL) {
        return sys_close(fd);
    }

    // if file is not in cache close only source file
    if (cache[fd]->c_cache == -1) {
        free(cache[fd]);
        cache[fd] = NULL;
        return sys_close(fd);
    }

    // if file in cache has been modified write changes to disk
    if (cache[fd]->c_lower != -1 && cache[fd]->c_upper != -1) {
        copy_data_interval(cache[fd]->c_cache, fd, cache[fd]->c_lower, (cache[fd]->c_upper - cache[fd]->c_lower));
    }

    // close cache file
	sys_close(cache[fd]->c_cache);

    // clear cache struct on file close
    free(cache[fd]);
    cache[fd] = NULL;

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