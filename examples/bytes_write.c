#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/stat.h>

#define MIN_WAIT_TIME 50
#define MAX_WAIT_TIME 100

#define MIN_READ_SIZE 1024
#define MAX_READ_SIZE 65536

//#define SOURCE_PATH "/mnt/nfs4"
#define SOURCE_PATH "/media/quaeck/storage"
//#define SOURCE_PATH "/home/quaeck/CLionProjects/ld-preload-benchmark/storage"

void benchmark(int files, int iterations, unsigned long * total_bytes, unsigned long * total_waiting) {

    int file, index, waiting;
    char path[PATH_MAX], data[MAX_READ_SIZE];

    *total_waiting = 0;
    *total_bytes = 0;

    srand(clock());

    for (int i = 0; i < iterations; i++) {

        if (i != 0) {
            printf("\033[F");
            printf("\033[K");
            fflush(stdout);
        }

        index = rand() % files;
        sprintf(path, "%s/folder%d/subfolder%d/file%d.txt", SOURCE_PATH, (index / (25 * 25)) % 25, (index / 25) % 25, index % 25);

        file = open(path, O_RDONLY);
        if (file == -1) {
            perror("ERROR: could not open file!");
            exit(EXIT_FAILURE);
        }

        struct stat _stat;
        fstat(file, &_stat);

        int offset = rand() % _stat.st_size;
        int size = MIN_READ_SIZE + (rand() % (MAX_READ_SIZE - MIN_READ_SIZE));

        lseek(file, offset, SEEK_SET);
        *total_bytes += read(file, &data, size);

        waiting = MIN_WAIT_TIME + (rand() % (MAX_WAIT_TIME - MIN_WAIT_TIME));
        usleep(waiting);
        *total_waiting += waiting;

        close(file);

        printf("completed:      %05d/%05d\n", i+1, iterations);
    }
}

int main() {

    int files = 250;
    int iterations = 2500;

    unsigned long total_bytes, total_waiting;

    struct timespec tstart={0,0}, tend={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);

    //----------------------------------------------------------------------------------------------

    for (int i = 0; i < 1; i++) {
        benchmark(files, iterations, &total_bytes, &total_waiting);
    }

    //----------------------------------------------------------------------------------------------

    clock_gettime(CLOCK_MONOTONIC, &tend);

    double time_total = ((double) tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double) tstart.tv_sec + 1.0e-9 * tstart.tv_nsec);
    double time_waited = (double) total_waiting / 1000000.0;
    double time_data = time_total - time_waited;
    double time_per_byte = (time_data / total_bytes) * 1000000;

    printf("bytes read:     %lu\n", total_bytes);
    printf("time used:      %.5f\n", time_total);
    printf("time waited:    %.5f\n", time_waited);
    printf("time data:      %.5f\n", time_data);
    printf("time per byte:  %.5f\n", time_per_byte);
}