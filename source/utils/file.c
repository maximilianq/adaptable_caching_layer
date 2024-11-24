#define _GNU_SOURCE

#include "file.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "../calls.h"

#define BLOCK_SIZE 4096
#define min(a, b) ((a) < (b) ? (a) : (b))

char * data_buffer = NULL;


void copy_file_interval(char * source, char * target, ssize_t start, ssize_t end) {

    void * data = malloc(sizeof(char) * BLOCK_SIZE);
    ssize_t bytes_read, bytes_written, bytes_total = 0;

    ssize_t size = end - start;

    int source_file = sys_open(source, O_RDONLY, 0);
    if (source_file == -1) {
        perror("ERROR: could not open source file!");
        exit(EXIT_FAILURE);
    }

    int target_file = sys_open(target, O_WRONLY, 0);
    if (target_file == -1) {
        perror("ERROR: could not open target file!");
        exit(EXIT_FAILURE);
    }

    if (lseek(source_file, start, SEEK_SET) == -1) {
        perror("ERROR: could not seek source file!");
        exit(EXIT_FAILURE);
    }

    if (lseek(target_file, start, SEEK_SET) == -1) {
        perror("ERROR: could not seek target file!");
        exit(EXIT_FAILURE);
    }

    do {

	
        if ((bytes_read = sys_read(source_file, data, min(size - bytes_total, BLOCK_SIZE))) == -1) {
            perror("ERROR: could not read source file!");
            exit(EXIT_FAILURE);
        }

        if (bytes_read <= 0) {
            break;
        }

        if ((bytes_written = sys_write(target_file, data, BLOCK_SIZE)) == -1) {
            perror("ERROR: could not write target file!");
            exit(EXIT_FAILURE);
        }

	//printf("~~~~~> %d %d\n", bytes_read, bytes_written);
	
        //if (bytes_read != bytes_written) {
        //    perror("ERROR: could not copy bytes from source to target!");
        //    exit(EXIT_FAILURE);
        //}
	

        bytes_total += bytes_read;

    } while (bytes_total < start);

    // Get the timestamps of the source file
    struct stat source_stat;
    if (fstat(source_file, &source_stat) == -1) {
        perror("Failed to get source file stats");
        exit(EXIT_FAILURE);
    }

    // Set the timestamps of the cache file
    struct timespec target_times[2];
    target_times[0] = source_stat.st_atim;
    target_times[1] = source_stat.st_mtim;
    if (futimens(target_file, target_times) == -1) {
        perror("Failed to target file timestamps");
        exit(EXIT_FAILURE);
    }

    sys_close(source_file);
    sys_close(target_file);
    free(data);
}


void copy_data(int source_file, int cache_file) {

    if (data_buffer == NULL) {
        if (posix_memalign((void **) &data_buffer, 4096, BLOCK_SIZE * 16) != 0) {
            perror("Error allocating aligned memory\n");
            exit(EXIT_FAILURE);
        }
    }

    ssize_t bytes_read, bytes_written, bytes_total;

    bytes_total = 0;

    lseek(source_file, 0, SEEK_SET);
    lseek(cache_file, 0, SEEK_SET);

    while ((bytes_read = sys_read(source_file, data_buffer, BLOCK_SIZE * 2)) > 0) {
        bytes_written = sys_write(cache_file, data_buffer, BLOCK_SIZE * 2);
        if (bytes_written == -1) {
            perror("Error writing to destination file");
            exit(EXIT_FAILURE);
        }

        bytes_total += bytes_read;
    }

    if (ftruncate(cache_file, bytes_total) == -1) {
        perror("Error truncating cache file");
        exit(EXIT_FAILURE);
    }
}

void copy_file(const char * source_path, const char * cache_path) {

    int source_file, cache_file;

    if ((source_file = sys_open(source_path, O_RDONLY | O_DIRECT, 0)) == -1) {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    if ((cache_file = sys_open(cache_path, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, S_IRWXU)) == -1) {
        perror("Error opening destination file");
        exit(EXIT_FAILURE);
    }

    copy_data(source_file, cache_file);

    // Get the timestamps of the source file
    struct stat source_stat;
    if (fstat(source_file, &source_stat) == -1) {
        perror("Failed to get source file stats");
        exit(EXIT_FAILURE);
    }

    // Set the timestamps of the cache file
    struct timespec cache_times[2];
    cache_times[0] = source_stat.st_atim;
    cache_times[1] = source_stat.st_mtim;
    if (futimens(cache_file, cache_times) == -1) {
        perror("Failed to set destination file timestamps");
        exit(EXIT_FAILURE);
    }

    sys_close(source_file);
    sys_close(cache_file);
}
