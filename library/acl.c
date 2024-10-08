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
#include "cache.h"
#include "memory.h"

#include "utils/file.h"
#include "utils/path.h"

#include "structures/queue.h"

#include "strategies/strategies.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

memory_t * global_memory = NULL;

pthread_t thread_lookahead;

void acl_init() {

	// initialize memory
	global_memory = malloc(sizeof(memory_t));
	init_memory(global_memory);

	// create lookahead thread
	pthread_create(&thread_lookahead, NULL, handle_lookahead, global_memory);
}

int acl_advise(const char * path, int flags) {

	if (global_memory == NULL)
		acl_init();

	char * source_path = malloc(PATH_MAX * sizeof(char));
	get_full_path(path, source_path, PATH_MAX);

	// enqueuing the advised path in priority queue
	enqueue(&global_memory->m_queue_high, source_path);

	return 1;
}

int acl_open(const char * path, int flags, mode_t mode) {

	//printf("[OPEN] %s %d %d\n", path, flags, mode);

	if (global_memory == NULL)
		acl_init();

	// expand path to full path
	char * source_path = malloc(PATH_MAX * sizeof(char));
	get_full_path(path, source_path, PATH_MAX);

	// open source file
	int source_file = sys_open(source_path, flags, mode);
	if (source_file == -1) {
		return source_file;
	}

	// check if file is in cache
	cache_entry_t * entry = retrieve_cache(&global_memory->m_cache, source_path);

	pthread_mutex_lock(&global_memory->m_mapping_mutex[source_file]);

	// add data to mapping information
	global_memory->m_mapping_entries[source_file] = malloc(sizeof(mapping_entry_t));
	strcpy(global_memory->m_mapping_entries[source_file]->me_path, source_path);
	global_memory->m_mapping_entries[source_file]->me_entry = entry;
	global_memory->m_mapping_entries[source_file]->me_descriptor = -1;
	global_memory->m_mapping_entries[source_file]->me_flags = flags;
	global_memory->m_mapping_entries[source_file]->me_mode = mode;
	global_memory->m_mapping_entries[source_file]->me_lower = -1;
	global_memory->m_mapping_entries[source_file]->me_upper = -1;

	if (entry == NULL) {
		pthread_mutex_unlock(&global_memory->m_mapping_mutex[source_file]);
		return source_file;
	}

	int cache_file = sys_open(entry->ce_cache, flags, mode);
	global_memory->m_mapping_entries[source_file]->me_descriptor = cache_file;

	pthread_mutex_unlock(&global_memory->m_mapping_mutex[source_file]);

    return source_file;
}

ssize_t acl_read(int fd, void * buffer, size_t count) {

	//printf("[READ] %d %lu\n", fd, count);

	if (global_memory == NULL)
		acl_init();

	pthread_mutex_lock(&global_memory->m_mapping_mutex[fd]);

	if (global_memory->m_mapping_entries[fd] == NULL) {
		pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);

		size_t result = sys_read(fd, buffer, count);

		return result;
	}

	if (global_memory->m_mapping_entries[fd]->me_entry == NULL) {

		pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);

		char * source_path = malloc(PATH_MAX * sizeof(char));
		strcpy(source_path, global_memory->m_mapping_entries[fd]->me_path);
		enqueue(&global_memory->m_queue_prefetch, source_path);

		size_t result = sys_read(fd, buffer, count);

		return result;
	}

	pthread_mutex_lock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);

	// get current offset of filedescriptor
	off_t offset = lseek(fd, 0, SEEK_CUR);
	if (offset == -1) {
		perror("ERROR: could not get offset of filedescriptor!");
		exit(EXIT_FAILURE);
	}

	// set offset of original file descriptor to cached file descriptor
	if (lseek(global_memory->m_mapping_entries[fd]->me_descriptor, offset, SEEK_SET) == -1) {
		perror("ERROR: could not set offset of cache file!");
		exit(EXIT_FAILURE);
	}

	// handle read using original read method
    ssize_t result = sys_read(global_memory->m_mapping_entries[fd]->me_descriptor, buffer, count);

	// adapt offset of original filedescriptor
	if (lseek(fd, result, SEEK_CUR) == -1) {
		perror("ERROR: could not set offset of source file!");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);
	pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);

	return result;
}

ssize_t acl_write(int fd, const void * buffer, size_t count) {

	//printf("[WRITE] %d %lu\n", fd, count);

	if (global_memory == NULL)
		acl_init();

	pthread_mutex_lock(&global_memory->m_mapping_mutex[fd]);

	if (global_memory->m_mapping_entries[fd] == NULL) {
		pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);
		size_t result = sys_write(fd, buffer, count);
		return result;
	}

	if (global_memory->m_mapping_entries[fd]->me_entry == NULL) {

		pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);

		char * source_path = malloc(PATH_MAX * sizeof(char));
		strcpy(source_path, global_memory->m_mapping_entries[fd]->me_path);
		enqueue(&global_memory->m_queue_prefetch, source_path);

		size_t result = sys_write(fd, buffer, count);

		return result;
	}

	pthread_mutex_lock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);

	// get current offset of filedescriptor
	off_t offset = lseek(fd, 0, SEEK_CUR);
	if (offset == -1) {
		perror("ERROR: could not get offset of filedescriptor!");
		exit(EXIT_FAILURE);
	}

	// set offset of original file descriptor to cached file descriptor
	if (lseek(global_memory->m_mapping_entries[fd]->me_descriptor, offset, SEEK_SET) == -1) {
		perror("ERROR: could not set offset of cache file!");
		exit(EXIT_FAILURE);
	}

	// handle read using original read method
	ssize_t result = sys_write(global_memory->m_mapping_entries[fd]->me_descriptor, buffer, count);

	// adapt offset of original filedescriptor
	if (lseek(fd, result, SEEK_CUR) == -1) {
		perror("ERROR: could not set offset of source file!");
		exit(EXIT_FAILURE);
	}

	// calculate modification bound of current request
	ssize_t lower = offset;
	ssize_t upper = offset + result;

	// experimentally add to extent list
	push_extent(&global_memory->m_mapping_entries[fd]->me_entry->ce_extents, lower, upper);

	pthread_mutex_unlock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);
	pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);

	return result;
}

int acl_close(int fd) {

	if (global_memory == NULL)
		acl_init();

	pthread_mutex_lock(&global_memory->m_mapping_mutex[fd]);

	if (global_memory->m_mapping_entries[fd] != NULL && global_memory->m_mapping_entries[fd]->me_entry != NULL) {

		sys_close(global_memory->m_mapping_entries[fd]->me_descriptor);

		pthread_mutex_lock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);

		ssize_t upper, lower;
		while (pop_extent(&global_memory->m_mapping_entries[fd]->me_entry->ce_extents, &lower, &upper) != -1) {
			copy_file_interval(global_memory->m_mapping_entries[fd]->me_entry->ce_cache, global_memory->m_mapping_entries[fd]->me_entry->ce_source, lower, upper);
		}

		pthread_mutex_unlock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);

		free(global_memory->m_mapping_entries[fd]);
		global_memory->m_mapping_entries[fd] = NULL;
	}

	pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);

    return sys_close(fd);
}

int acl_sync(int fd) {

	printf("[SYNC] %d\n", fd);

	if (global_memory == NULL)
		acl_init();

	pthread_mutex_lock(&global_memory->m_mapping_mutex[fd]);

	if (global_memory->m_mapping_entries[fd] != NULL && global_memory->m_mapping_entries[fd]->me_entry != NULL) {

		pthread_mutex_lock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);

		ssize_t upper, lower;
		while (pop_extent(&global_memory->m_mapping_entries[fd]->me_entry->ce_extents, &lower, &upper) != -1) {
			copy_file_interval(global_memory->m_mapping_entries[fd]->me_entry->ce_cache, global_memory->m_mapping_entries[fd]->me_entry->ce_source, lower, upper);

		}

		pthread_mutex_unlock(&global_memory->m_mapping_entries[fd]->me_entry->ce_mutex);
	}

	pthread_mutex_unlock(&global_memory->m_mapping_mutex[fd]);

	return sys_sync(fd);
}