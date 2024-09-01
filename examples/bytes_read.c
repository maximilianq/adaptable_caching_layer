#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../library/constants.h"

#include "../library/external/acl.h"

#define MIN_WAIT_TIME 1000
#define MAX_WAIT_TIME 2000

// Ensure minimum read size and alignment to 4096 bytes
#define ALIGNMENT 4096
#define MIN_READ_SIZE 4096
#define MAX_READ_SIZE 262144

void benchmark(int files, int iterations, unsigned long * total_bytes, unsigned long * total_waiting) {


    int file, index;
    char path[PATH_MAX];

    *total_waiting = 0;
    *total_bytes = 0;

    srand(clock());

    for (int i = 0; i < iterations; i++) {

        // get file path of a random file
        //index = i / (iterations / files);
        index = rand() % files;
        sprintf(path, "%s/folder%d/subfolder%d/file%d.txt", SOURCE_PATH, (index / (25 * 25)) % 25, (index / 25) % 25, index % 25);

        if (rand() % 100 == 0)
            ladvise(path, 1);

        file = open(path, O_RDONLY | O_DIRECT);
        if (file == -1) {
            perror("ERROR: could not open test file!");
            exit(EXIT_FAILURE);
        }

        // Get the file size
        struct stat file_stat;
        if (fstat(file, &file_stat) == -1) {
            perror("Error getting file stats\n");
            exit(EXIT_FAILURE);
        }

        off_t file_size = file_stat.st_size;

        // Generate a random offset and read size
        off_t offset = (rand() % (file_size / ALIGNMENT)) * ALIGNMENT;
        size_t size = ((MIN_READ_SIZE + (rand() % (MAX_READ_SIZE - MIN_READ_SIZE))) / ALIGNMENT) * ALIGNMENT;

        // Ensure the read does not go beyond the file size
        if (offset + size > file_size) {
            size = file_size - offset;
            size = (size / ALIGNMENT) * ALIGNMENT;  // Adjust size to be a multiple of ALIGNMENT
        }

        // Allocate aligned buffer for reading
        void * buffer;
        if (posix_memalign(&buffer, ALIGNMENT, size) != 0) {
            perror("Error allocating aligned memory\n");
            exit(EXIT_FAILURE);
        }

        // Perform the read operation
        if (lseek(file, offset, SEEK_SET) == -1) {
            perror("Error seeking to offset\n");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read = read(file, buffer, size);
        if (bytes_read == -1) {
            perror("Error reading file\n");
            exit(EXIT_FAILURE);
        }

        *total_bytes += bytes_read;

        // wait a ranfom amount of micro seconds
        //waiting = MIN_WAIT_TIME + (rand() % (MAX_WAIT_TIME - MIN_WAIT_TIME));
        //usleep(waiting);
        //*total_waiting += waiting;

        // Clean up
        free(buffer);
        close(file);

        if (i != 0) {
            printf("\033[F");
            printf("\033[K");
            fflush(stdout);
        }

        printf("completed:      %06d/%06d\n", i+1, iterations);
    }
}

int main() {

    int files = 625;
    int iterations = 62500;

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