#define _GNU_SOURCE

#include "file.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "calls.h"

#define BLOCK_SIZE 131072
#define min(a, b) ((a) < (b) ? (a) : (b))

void copy_data_interval(int source, int target, off_t offset, size_t size) {

    char data[BLOCK_SIZE];
    ssize_t bytes_read, bytes_written, bytes_total = 0;

    if (lseek(source, offset, SEEK_SET) == -1) {
        perror("ERROR: could not seek source file!");
        exit(EXIT_FAILURE);
    }

    if (lseek(target, offset, SEEK_SET) == -1) {
        perror("ERROR: could not seek target file!");
        exit(EXIT_FAILURE);
    }

    do {
        if ((bytes_read = sys_read(source, data, min(size - bytes_total, BLOCK_SIZE))) == -1) {
            perror("ERROR: could not read source file!");
            exit(EXIT_FAILURE);
        }

        if (bytes_read == 0) {
            break;
        }

        if ((bytes_written = write(target, data, bytes_read)) == -1) {
            perror("ERROR: could not write target file!");
            exit(EXIT_FAILURE);
        }

        if (bytes_read != bytes_written) {
            perror("ERROR: could not copy bytes from source to target!");
            exit(EXIT_FAILURE);
        }

        bytes_total += bytes_read;

    } while (bytes_total < size);
}

void copy_data(int source_file, int cache_file) {

    char data[131072];
    ssize_t bytes_read, bytes_written;

    lseek(source_file, 0, SEEK_SET);
    lseek(cache_file, 0, SEEK_SET);

    while ((bytes_read = sys_read(source_file, data, 131072)) > 0) {
        bytes_written = write(cache_file, data, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            exit(EXIT_FAILURE);
        }
    }
}


void copy_file(const char * source_path, const char * cache_path) {

    int source_file, cache_file;

    if ((source_file = sys_open(source_path, O_RDONLY, 0)) == -1) {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    if ((cache_file = sys_open(cache_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) == -1) {
        perror("Error opening destination file");
        exit(EXIT_FAILURE);
    }

    copy_data(source_file, cache_file);

    sys_close(source_file);
    sys_close(cache_file);
}
