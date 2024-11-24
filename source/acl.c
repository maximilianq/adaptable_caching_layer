#define _GNU_SOURCE

#include "acl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

#include "calls.h"
#include "mapping.h"
#include "prefetch.h"
#include "cache.h"

#include "utils/file.h"
#include "utils/path.h"

#include "structures/queue.h"

#include "prefetch/variant.h"
#include "cache/variant.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

cache_t * cache = NULL;
prefetch_t * prefetch = NULL;
mapping_t * mapping = NULL;

pthread_t thread_lookahead;

void acl_import(cache_t * cache) {

	const char * cache_location = getenv("ACL_CACHE_PATH");
	if (cache_location == NULL) {
		perror("Error: could not retrieve cache path!");
		exit(EXIT_FAILURE);
	}

	struct dirent * directory_entry;
	DIR * directory = opendir(cache_location);

	if (directory == NULL) {
		perror("Error: could not open cache directory.");
		return;
	}

	while ((directory_entry = readdir(directory)) != NULL) {

		// Skip the current and parent directory entries
		if (strcmp(directory_entry->d_name, ".") == 0 || strcmp(directory_entry->d_name, "..") == 0) {
			continue;
		}

		// Construct the full file name
		char filename[PATH_MAX];
		strcpy(filename, directory_entry->d_name);
		for (char *p = filename; *p; p++) {
			if (*p == '_') {
				*p = '/';
			}
		}

		// Prepare the modified filename
		char path[PATH_MAX];
		strcpy(path, SOURCE_PATH);
		strcat(path, "/");
		strcat(path, filename);

		if (access(path, F_OK) == 0)
                    insert_cache(cache, path);
	}

	closedir(directory);
}

void acl_init() {

	if (mapping == NULL) {
		mapping = malloc(sizeof(mapping_t));
		mapping_init(mapping);
	}

	// initialize cache structure
	if (cache == NULL) {

		// allocate memory for storing cache structure
		cache = malloc(sizeof(cache_t));

		// retrieve cache capacity from environmental variable
		const char * capacity_string = getenv("ACL_CAPACITY");
		ssize_t capacity = 0;
		if (capacity_string == NULL) {
			perror("Error: could not retrieve cache capacity!");
			exit(EXIT_FAILURE);
		}
		capacity = strtol(capacity_string, NULL, 10);

		// retrieve cache policy from environmental variable
		const char * policy = getenv("ACL_POLICY");
		if (policy == NULL) {
			perror("Error: could not retrieve caching policy!");
			exit(EXIT_FAILURE);
		}

		cache_policy_t cache_policy;
		if (strcmp(policy, "FIFO") == 0) {
			cache_policy = policy_fifo;
		} else if (strcmp(policy, "LRU") == 0) {
			cache_policy = policy_lru;
		} else if (strcmp(policy, "LFU") == 0) {
			cache_policy = policy_lfu;
		} else {
			perror("Error: the specified cache policy is not implemented!");
			exit(EXIT_FAILURE);
		}

		init_cache(cache, mapping, capacity, cache_policy, mapping_cache_inserted, mapping_cache_removed);
	}

	// initialize prefetch structure
	if (prefetch == NULL) {

		// allocate memory for storing prefetch structure
		prefetch = malloc(sizeof(prefetch_t));

		// retrieve prefetch strategy from environmental variable
		const char * strategy = getenv("ACL_STRATEGY");
		if (strategy == NULL) {
			perror("Error: could not retrieve prefetch strategy!");
			exit(EXIT_FAILURE);
		}

		prefetch_strategy_t prefetch_strategy;
		if (strcmp(strategy, "ABFP") == 0) {
			prefetch_strategy = abfp_strategy;
		} else if (strcmp(strategy, "FSDP") == 0) {
			prefetch_strategy = fsdp_strategy;
		} else if (strcmp(strategy, "MCFP") == 0) {
			prefetch_strategy = mcfp_strategy;
		} else if (strcmp(strategy, "NONE") == 0) {
			prefetch_strategy.ps_init = NULL;
			prefetch_strategy.ps_free = NULL;
			prefetch_strategy.ps_process = NULL;
		} else {
			perror("Error: the specified prefetch strategy is not implemented!");
			exit(EXIT_FAILURE);
		}

		prefetch_init(prefetch, cache, prefetch_strategy);
	}

	acl_import(cache);
}

int acl_advise(const char * path, int flags) {

	// expand relative path to full path
	char * source_path = malloc(PATH_MAX * sizeof(char));
	get_full_path(path, source_path);

	// check if file will be needed in the future and push_queue into high priority caching
	if ((flags & ACL_WILLNEED) == ACL_WILLNEED) {
		push_queue(&prefetch->p_high, source_path);
	}

	// check if file will not be needed in the future and remove from cache
	if ((flags & ACL_DONTNEED) == ACL_DONTNEED) {
		remove_cache(cache, source_path);
	}

	return 0;
}

void acl_prefetch(prefetch_strategy_t strategy) {
	prefetch_replace(prefetch, cache, strategy);
}

void acl_cache(cache_policy_t policy) {
	cache_replace(cache, policy);
}

void acl_select(prefetch_t * prefetch, char * path) {
	push_queue(&prefetch->p_low, path);
}

int acl_miss(prefetch_t * prefetch, char * path, int operation) {

	cache_miss_t * cache_miss = malloc(sizeof(cache_miss_t));

	strcpy(cache_miss->cm_path, path);
	cache_miss->cm_time = time(NULL);
	cache_miss->cm_operation = operation;

	push_queue(&prefetch->p_history, cache_miss);
}

int acl_open(const char * path, int flags, mode_t mode) {

	int source_file, cache_file = -1;
	cache_entry_t * cache_entry;

	// open source file and return prematurely if file could not be opened
	if ((source_file = sys_open(path, flags, mode)) == -1) {
		return source_file;
	}

	pthread_mutex_lock(&cache->c_mutex);

	// check if file is already in cache and if yes open cache file
	if ((cache_entry = retrieve_cache(cache, (char *) path)) != NULL) {
		if ((cache_file = sys_open(cache_entry->ce_cache, flags, mode)) == -1) {
			cache_entry = NULL;
		}
	}

	pthread_mutex_unlock(&cache->c_mutex);

	// lock mapping mutex for source file descriptor
	pthread_mutex_lock(&mapping->m_mutex[source_file]);

	// allocate state and add data to mapping information
	mapping->m_entries[source_file] = malloc(sizeof(mapping_entry_t));
	strcpy(mapping->m_entries[source_file]->me_path, path);
	mapping->m_entries[source_file]->me_entry = cache_entry;
	mapping->m_entries[source_file]->me_descriptor = cache_file;
	mapping->m_entries[source_file]->me_flags = flags;
	mapping->m_entries[source_file]->me_mode = mode;
	mapping->m_entries[source_file]->me_hinted = 0;

	// release mutex of mapping
	pthread_mutex_unlock(&mapping->m_mutex[source_file]);

	// return source file filedescriptor
    return source_file;
}

ssize_t acl_read(int fd, void * buffer, size_t count) {

	pthread_mutex_lock(&mapping->m_mutex[fd]);

	// check if object was opened through caching layer
        if (mapping->m_entries[fd] == NULL) {
                pthread_mutex_unlock(&mapping->m_mutex[fd]);
                return sys_read(fd, buffer, count);
        }

	// check if item is in cache and if not push_queue in prefetch queue
	if (mapping->m_entries[fd]->me_entry == NULL) {

		if (mapping->m_entries[fd]->me_hinted == 0) {
			mapping->m_entries[fd]->me_hinted = 1;
			acl_miss(prefetch, mapping->m_entries[fd]->me_path, CM_READ);
		}

		pthread_mutex_unlock(&mapping->m_mutex[fd]);

		return sys_read(fd, buffer, count);
	}

	// check if item is currently being transferred and if yes revert to source file
	if (pthread_mutex_trylock(&mapping->m_entries[fd]->me_entry->ce_mutex) != 0) {
                pthread_mutex_unlock(&mapping->m_mutex[fd]);
                return sys_read(fd, buffer, count);
	}

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

	pthread_mutex_lock(&mapping->m_mutex[fd]);

	if (mapping->m_entries[fd] == NULL) {
		pthread_mutex_unlock(&mapping->m_mutex[fd]);
		ssize_t result = sys_write(fd, buffer, count);

		return result;
	}

	if (mapping->m_entries[fd]->me_entry == NULL) {

		if (mapping->m_entries[fd]->me_hinted == 0) {
			mapping->m_entries[fd]->me_hinted = 1;
			acl_miss(prefetch, mapping->m_entries[fd]->me_path, CM_WRITE);
		}

		pthread_mutex_unlock(&mapping->m_mutex[fd]);

		return sys_write(fd, buffer, count);
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

	return result;
}

int acl_close(int fd) {

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

    return sys_close(fd);
}

int acl_sync(int fd) {

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

	return sys_sync(fd);
}