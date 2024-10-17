
#define _GNU_SOURCE

#include "acl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <pthread.h>

#include "calls.h"

#include "mapping.h"
#include "prefetch.h"
#include "cache.h"

#include "utils/file.h"
#include "utils/path.h"

#include "structures/queue.h"

//#include "prefetch/mcfl.h"
#include "prefetch/fsdl.h"
#include "cache/lfu.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct mapping mapping_t;

cache_t * cache = NULL;
prefetch_t * prefetch = NULL;
mapping_t * mapping = NULL;

pthread_t thread_lookahead;

void acl_init() {

	if (mapping == NULL) {
		mapping = malloc(sizeof(mapping_t));
		mapping_init(mapping);
	}

	if (cache == NULL) {
		cache = malloc(sizeof(cache_t));
		init_cache(cache, CACHE_SIZE, mapping_cache_inserted, mapping, mapping_cache_removed, mapping, insert_lfu, update_lfu);
	}

	if (prefetch == NULL) {
		prefetch = malloc(sizeof(prefetch_t));
		prefetch_init(prefetch, NULL, process_fsdl, NULL);
	}

	arguments_t * arguments = malloc(sizeof(arguments_t));
	arguments->a_cache = cache;
	arguments->a_prefetch = prefetch;

	pthread_create(&prefetch->p_thread, NULL, prefetch_handler, arguments);
}

int ACL_WILLNEED = 1;
int ACL_DONTNEED = 2;

int acl_advise(const char * path, int flags) {

	// expand relative path to full path
	char * source_path = malloc(PATH_MAX * sizeof(char));
	get_full_path(path, source_path, PATH_MAX);

	// check if file will be needed in the future and enqueue into high priority caching
	if ((flags & ACL_WILLNEED) == ACL_WILLNEED) {
		enqueue(&prefetch->p_high, source_path);
	}

	// check if file will not be needed in the future and remove from cache
	if ((flags & ACL_DONTNEED) == ACL_DONTNEED) {
		remove_cache(cache, source_path);
	}

	return 0;
}

void acl_prefetch(void * init, void * process, void * free) {
	prefetch_replace(prefetch, cache, init, process, free);
}

void acl_select(void * prefetch, char * path) {
	enqueue(&((prefetch_t *) prefetch)->p_low, path);
}

int acl_open(const char * path, int flags, mode_t mode) {

	int source_file, cache_file = -1;
	cache_entry_t * cache_entry;

	// open source file and return prematurely if file could not be opened
	if ((source_file = sys_open(path, flags, mode)) == -1) {
		return source_file;
	}

	// check if file is already in cache and if yes open cache file
	if ((cache_entry = retrieve_cache(cache, (char *) path)) != NULL) {

		pthread_mutex_lock(&cache_entry->ce_mutex);

		struct stat source_stat;
		if (lstat(cache_entry->ce_source, &source_stat) == -1) {
			perror("ERROR: could not stat cache file!\n");
			exit(EXIT_FAILURE);
		}

		if (cache_entry->ce_mtime != source_stat.st_mtime || cache_entry->ce_size != source_stat.st_size) {
			printf("[WARNING] Cached file was modified thus any changes will be ignored!");
			remove_cache(cache, (char *) path);
		}

		if ((cache_file = sys_open(cache_entry->ce_cache, flags, mode)) == -1) {
			perror("ERROR: could not open cache file!\n");
			exit(EXIT_FAILURE);
		}

		pthread_mutex_unlock(&cache_entry->ce_mutex);
	} else {

                char * source_path = malloc(PATH_MAX * sizeof(char));
                strcpy(source_path, path);
                enqueue(&prefetch->p_history, source_path);
	}

	// lock mapping mutex for source file descriptor
	pthread_mutex_lock(&mapping->m_mutex[source_file]);

	// allocate state and add data to mapping information
	mapping->m_entries[source_file] = malloc(sizeof(mapping_entry_t));
	strcpy(mapping->m_entries[source_file]->me_path, path);
	mapping->m_entries[source_file]->me_entry = cache_entry;
	mapping->m_entries[source_file]->me_descriptor = cache_file;
	mapping->m_entries[source_file]->me_flags = flags;
	mapping->m_entries[source_file]->me_mode = mode;

	// release mutex of mapping
	pthread_mutex_unlock(&mapping->m_mutex[source_file]);

	// return source file filedescriptor
    return source_file;
}

ssize_t acl_read(int fd, void * buffer, size_t count) {

	pthread_mutex_lock(&mapping->m_mutex[fd]);

	if (mapping->m_entries[fd] == NULL) {
		pthread_mutex_unlock(&mapping->m_mutex[fd]);

		ssize_t result = sys_read(fd, buffer, count);

		return result;
	}

	if (mapping->m_entries[fd]->me_entry == NULL) {

		pthread_mutex_unlock(&mapping->m_mutex[fd]);

		//char * source_path = malloc(PATH_MAX * sizeof(char));
		//strcpy(source_path, mapping->m_entries[fd]->me_path);
		//enqueue(&prefetch->p_history, source_path);

		ssize_t result = sys_read(fd, buffer, count);

		return result;
	}

	pthread_mutex_lock(&mapping->m_entries[fd]->me_entry->ce_mutex);

	// get current offset of filedescriptor
	off_t offset = lseek(fd, 0, SEEK_CUR);
	if (offset == -1) {
		perror("ERROR: could not get offset of filedescriptor!");
		exit(EXIT_FAILURE);
	}

	// set offset of original file descriptor to cached file descriptor
	if (lseek(mapping->m_entries[fd]->me_descriptor, offset, SEEK_SET) == -1) {
		perror("ERROR: could not set offset of cache file!");
		exit(EXIT_FAILURE);
	}

	// handle read using original read method
        ssize_t result = sys_read(mapping->m_entries[fd]->me_descriptor, buffer, count);

	if (lseek(fd, result, SEEK_CUR) == -1) {
		perror("ERROR: could not set offset of source file!");
		exit(EXIT_FAILURE);
	}


	pthread_mutex_unlock(&mapping->m_entries[fd]->me_entry->ce_mutex);
	pthread_mutex_unlock(&mapping->m_mutex[fd]);

	return result;
}

ssize_t acl_write(int fd, const void * buffer, size_t count) {

	printf("<write>\n");

	pthread_mutex_lock(&mapping->m_mutex[fd]);

	if (mapping->m_entries[fd] == NULL) {
		pthread_mutex_unlock(&mapping->m_mutex[fd]);
		ssize_t result = sys_write(fd, buffer, count);

		printf("</write>\n");

		return result;
	}

	if (mapping->m_entries[fd]->me_entry == NULL) {

		pthread_mutex_unlock(&mapping->m_mutex[fd]);

		//char * source_path = malloc(PATH_MAX * sizeof(char));
		//strcpy(source_path, mapping->m_entries[fd]->me_path);
		//enqueue(&prefetch->p_history, source_path);

		ssize_t result = sys_write(fd, buffer, count);

		printf("</write>\n");


		return result;
	}

	pthread_mutex_lock(&mapping->m_entries[fd]->me_entry->ce_mutex);

	// get current offset of filedescriptor
	off_t offset = lseek(fd, 0, SEEK_CUR);
	if (offset == -1) {
		perror("ERROR: could not get offset of filedescriptor!");
		exit(EXIT_FAILURE);
	}

	// set offset of original file descriptor to cached file descriptor
	if (lseek(mapping->m_entries[fd]->me_descriptor, offset, SEEK_SET) == -1) {
		perror("ERROR: could not set offset of cache file!");
		exit(EXIT_FAILURE);
	}

	// handle read using original read method
	ssize_t result = sys_write(mapping->m_entries[fd]->me_descriptor, buffer, count);

	// adapt offset of original filedescriptor
	if (lseek(fd, result, SEEK_CUR) == -1) {
		perror("ERROR: could not set offset of source file!");
		exit(EXIT_FAILURE);
	}

	// calculate modification bound of current request
	ssize_t lower = offset;
	ssize_t upper = offset + result;

	// experimentally add to extent list
	push_extent(&mapping->m_entries[fd]->me_entry->ce_extents, lower, upper);

	pthread_mutex_unlock(&mapping->m_entries[fd]->me_entry->ce_mutex);
	pthread_mutex_unlock(&mapping->m_mutex[fd]);

	printf("</write>\n");

	return result;
}

int acl_close(int fd) {

        printf("<close>\n");

	pthread_mutex_lock(&mapping->m_mutex[fd]);

	if (mapping->m_entries[fd] != NULL && mapping->m_entries[fd]->me_entry != NULL) {

		sys_close(mapping->m_entries[fd]->me_descriptor);

		pthread_mutex_lock(&mapping->m_entries[fd]->me_entry->ce_mutex);

		ssize_t upper, lower;
		while (pop_extent(&mapping->m_entries[fd]->me_entry->ce_extents, &lower, &upper) != -1) {
			copy_file_interval(mapping->m_entries[fd]->me_entry->ce_cache, mapping->m_entries[fd]->me_entry->ce_source, lower, upper);
		}

		pthread_mutex_unlock(&mapping->m_entries[fd]->me_entry->ce_mutex);

	}

	if (mapping->m_entries[fd] != NULL) {
	   	free(mapping->m_entries[fd]);
                mapping->m_entries[fd] = NULL;
	}

	pthread_mutex_unlock(&mapping->m_mutex[fd]);

    printf("</close>\n");

    return sys_close(fd);
}

int acl_sync(int fd) {

	printf("<sync>\n");

	pthread_mutex_lock(&mapping->m_mutex[fd]);

	if (mapping->m_entries[fd] != NULL && mapping->m_entries[fd]->me_entry != NULL) {

		pthread_mutex_lock(&mapping->m_entries[fd]->me_entry->ce_mutex);

		ssize_t upper, lower;
		while (pop_extent(&mapping->m_entries[fd]->me_entry->ce_extents, &lower, &upper) != -1) {
			copy_file_interval(mapping->m_entries[fd]->me_entry->ce_cache, mapping->m_entries[fd]->me_entry->ce_source, lower, upper);

		}

		pthread_mutex_unlock(&mapping->m_entries[fd]->me_entry->ce_mutex);
	}

	pthread_mutex_unlock(&mapping->m_mutex[fd]);

	printf("</sync>\n");

	return sys_sync(fd);
}
