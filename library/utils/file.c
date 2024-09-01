#define _GNU_SOURCE

#include "file.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../calls.h"

#define BLOCK_SIZE 8192
#define min(a, b) ((a) < (b) ? (a) : (b))

char * data_buffer = NULL;

void copy_file_interval(char * source, char * target, ssize_t start, ssize_t end) {

    char data[BLOCK_SIZE];
    ssize_t bytes_read, bytes_written, bytes_total = 0;

    ssize_t size = end - start;

    int source_file = sys_open(source, O_RDONLY | O_DIRECT, 0);
    if (source_file == -1) {
        perror("ERROR: could not open source file!");
        exit(EXIT_FAILURE);
    }

    int target_file = sys_open(target, O_WRONLY | O_DIRECT, 0);
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

        if (bytes_read == 0) {
            break;
        }

        if ((bytes_written = sys_write(target_file, data, bytes_read)) == -1) {
            perror("ERROR: could not write target file!");
            exit(EXIT_FAILURE);
        }

        if (bytes_read != bytes_written) {
            perror("ERROR: could not copy bytes from source to target!");
            exit(EXIT_FAILURE);
        }

        bytes_total += bytes_read;

    } while (bytes_total < start);
}

void copy_data(int source_file, int cache_file) {

    if (data_buffer == NULL) {
        if (posix_memalign((void **) &data_buffer, 4096, BLOCK_SIZE) != 0) {
            perror("Error allocating aligned memory\n");
            exit(EXIT_FAILURE);
        }
    }

    ssize_t bytes_read, bytes_written;

    lseek(source_file, 0, SEEK_SET);
    lseek(cache_file, 0, SEEK_SET);

    while ((bytes_read = sys_read(source_file, data_buffer, BLOCK_SIZE)) > 0) {
        bytes_written = sys_write(cache_file, data_buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            exit(EXIT_FAILURE);
        }
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

    sys_close(source_file);
    sys_close(cache_file);
}
