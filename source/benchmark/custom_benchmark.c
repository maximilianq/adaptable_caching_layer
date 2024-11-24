#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <linux/limits.h>

#include "../external/acl.h"

#define PATTERN_SEQUENTIAL_READ 0;
#define PATTERN_RANDOM_READ 1;
#define PATTERN_SEQUENTIAL_WRITE 2;
#define PATTERN_RANDOM_WRITE 3;

typedef struct {
  char * path; // file path template
  int pattern; // access pattern (e.g. sequential read, random write, etc.)
  int files; // number of files
  int size; // size of each file
  unsigned int seed; // seed for all required random number generators
  int interval; // size of unique access pattern interval
  int iterations; // number of operations performed on a single file before advancing to the next
  int runtime; // runtime of the benchmark
  int window; // disclosure window of future file access
  int blocksize; // size of individuall read or write operations
  int will; // disclosure of future file access
  int wont; // disclosure of future absence of file access
  int init; // generate required files
} settings_t;

typedef struct result {
  ssize_t r_bytes;
  long r_ops;
  long r_time;
} result_t;

// Function to print usage information
void print_usage(const char *prog_name) {
  printf("Usage: %s --path=PATH --pattern=PATTERN --files=FILES --size=SIZE --seed=SEED --will --wont --init\n", prog_name);
}

settings_t * parse_settings(int argc, char *argv[]) {

  settings_t * settings = malloc(sizeof(settings_t));
  settings->path = NULL;
  settings->pattern = 0;
  settings->files = 0;
  settings->size = 0;
  settings->seed = 0;
  settings->interval = 0;
  settings->iterations = 0;
  settings->runtime = 0;
  settings->window = 0;
  settings->blocksize = 0;
  settings->will = 0;
  settings->wont = 0;
  settings->init = 0;

  // Define long options
  static struct option options[] = {
    {"path", optional_argument, 0, 1},
    {"pattern", optional_argument, 0, 2},
    {"files", optional_argument, 0, 3},
    {"size", optional_argument, 0, 4},
    {"seed", optional_argument, 0, 5},
    {"will", no_argument, 0, 6},
    {"wont", no_argument, 0, 7},
    {"init", no_argument, 0, 8},
    {"runtime", optional_argument, 0, 9},
    {"window", optional_argument, 0, 10},
    {"blocksize", optional_argument, 0, 11},
    {"interval", optional_argument, 0, 12},
    {"iteration", optional_argument, 0, 13}
  };

  int option_index = 0;
  int opt;

  while ((opt = getopt_long(argc, argv, "", options, &option_index)) != -1) {
    if (opt == 1) {
        settings->path = optarg;
    } else if (opt == 2) {
      if (strcmp(optarg, "read") == 0) {
        settings->pattern = PATTERN_SEQUENTIAL_READ;
      } else if (strcmp(optarg, "write") == 0) {
        settings->pattern = PATTERN_SEQUENTIAL_WRITE;
      } else if (strcmp(optarg, "randread") == 0) {
        settings->pattern = PATTERN_RANDOM_READ;
      } else if (strcmp(optarg, "randwrite") == 0) {
        settings->pattern = PATTERN_RANDOM_WRITE;
      } else {
        printf("Error: unrecognized pattern: %s\n", optarg);
        exit(EXIT_FAILURE);
      }
    } else if (opt == 3) {
      settings->files = (int) strtol(optarg, NULL, 10);
    } else if (opt == 4) {
      settings->size = (int) strtol(optarg, NULL, 10);
    } else if (opt == 5) {
      settings->seed = (unsigned int) strtol(optarg, NULL, 10);
    } else if (opt == 6) {
      settings->will = 1;
    } else if (opt == 7) {
      settings->wont = 1;
    } else if (opt == 8) {
      settings->init = 1;
    } else if (opt == 9) {
      settings->runtime = (int) strtol(optarg, NULL, 10);
    } else if (opt == 10) {
      settings->window = (int) strtol(optarg, NULL, 10);
    } else if (opt == 11) {
      settings->blocksize = (int) strtol(optarg, NULL, 10);
    } else if (opt == 12) {
      settings->interval = (int) strtol(optarg, NULL, 10);
    } else if (opt == 13) {
      settings->iterations = (int) strtol(optarg, NULL, 10);
    } else {
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  return settings;
}

void create_files(settings_t * settings) {

  srand(settings->seed);

  char * buffer = malloc(settings->size * sizeof(char));
  for (int i = 0; i < settings->size; i++) {
    buffer[i] = (char) (rand() % 26 + 65);
  }

  for (int i = 0; i < settings->files; i++) {

    char path[PATH_MAX];
    snprintf(path, PATH_MAX, settings->path, i);

    int file = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (file == -1) {
      printf("-----> %s\n", path);
      perror("Error: could not create benchmark file.");
      exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(file, buffer, settings->size);
    if (bytes_written == -1) {
      perror("Error: could not write to benchmark file.");
      exit(EXIT_FAILURE);
    }

    if (close(file) == -1) {
      perror("Error: could not close benchmark file.");
      exit(EXIT_FAILURE);
    }
  }

  free(buffer);
  buffer = NULL;
}

int * compute_pattern(settings_t * settings) {

  int * selection = malloc(sizeof(int) * settings->interval);
  if (selection == NULL) {
    perror("Error: could not allocate selection.");
    exit(EXIT_FAILURE);
  }

  srand(settings->seed);

  for (int i = 0; i < settings->interval; i++) {
    selection[i] = rand() % settings->files;
  }

  return selection;
}

result_t * perform_benchmark(settings_t * settings) {

  // allocate result structure
  result_t * result = malloc(sizeof(result_t));
  result->r_bytes = 0;
  result->r_ops = 0;

  // Allocate aligned buffer for reading and writing
  char * buffer;
  if (posix_memalign((void **) &buffer, 4096, settings->blocksize) != 0) {
    perror("Error allocating aligned memory\n");
    exit(EXIT_FAILURE);
  }

  // fill buffer with random characters
  for (int i = 0; i < settings->blocksize; i++) {
    buffer[i] = (char) (rand() % 26 + 65);
  }

  // compute access pattern
  int * selection = compute_pattern(settings);

  // seed random number generator
  srand(settings->seed);

  // initialize pointer to files
  int current = 0, prefetch = 4;

  // initialize time measurements for nanosecond precision
  struct timespec nano_start={0,0}, nano_end={0,0};

  // init start time to be utilized in limiting the application runtime
  time_t start_time = time(NULL);
  while (time(NULL) < start_time + settings->runtime) {

    // if disclosure of upcoming file access is enabled perform the disclosure
    if (settings->will) {

      // fill up the prefetch queue according to the prefetch window
      while (prefetch != (current + settings->window) % settings->interval) {

        // build file path given the current index
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, settings->path, selection[prefetch]);

        acl_advise(path, ACL_WILLNEED);

        prefetch = (prefetch + 1) % settings->interval;
      }
    }

    char path[PATH_MAX];
    snprintf(path, PATH_MAX, settings->path, selection[current]);

    // measure starting time of read or write operation
    clock_gettime(CLOCK_MONOTONIC, &nano_start);

    // open file either for reading or writing
    int file;
    if ((settings->pattern & 2) == 0) {
      file = open(path, O_DIRECT | O_RDONLY);
    } else {
      file = open(path, O_DIRECT | O_WRONLY);
    }

    // check if file was succesfully operned
    if (file == -1) {
      perror("Error: could not open benchmark file.");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < settings->iterations; i++) {

      // if random workload was configured perform seeking to random location
      if ((settings->pattern & 1) == 1) {

        off_t offset = ((rand() % settings->size) / settings->blocksize) * settings->blocksize;

        if (lseek(file, offset, SEEK_SET) == -1) {
          perror("Error: could not seek benchmark file.");
          exit(EXIT_FAILURE);
        }
      }

      // either read or write the number of bytes specified by blocksize from or to the file
      ssize_t bytes;
      if ((settings->pattern & 2) == 0) {

        bytes = read(file, buffer, settings->blocksize);

        if (bytes == -1) {
          perror("Error: could not read benchmark data.");
          exit(EXIT_FAILURE);
        }
      } else {

        bytes = write(file, buffer, settings->blocksize);

        if (bytes == -1) {
          perror("Error: could not write benchmark data.");
          exit(EXIT_FAILURE);
        }
      }

      result->r_bytes += bytes;
      result->r_ops += 1;

      usleep(25);
    }

    // close file after performing the predefined number of operations on the selected file
    if (close(file) == -1) {
      perror("Error: could not close benchmark file.");
      exit(EXIT_FAILURE);
    }

    // measure ending time of read or write operation and add to total time
    clock_gettime(CLOCK_MONOTONIC, &nano_end);
    result->r_time += (nano_end.tv_sec * 1000000000 + nano_end.tv_nsec) - (nano_start.tv_sec * 1000000000 + nano_start.tv_nsec);

    // if disclosure of upcoming absence of file access is enabled perform the disclosure
    if (settings->wont) {
      int lookahead = current + 1;
      while (lookahead != (current + 2 * settings->window) % settings->interval) {

        if (selection[lookahead] == selection[current])
          break;

        lookahead = (lookahead + 1) % settings->interval;
      }

      if (lookahead == (current + 2 * settings->window) % settings->interval) {

        // build file path given the current index
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, settings->path, selection[prefetch]);

        printf("hello\n");

        acl_advise(path, ACL_DONTNEED);
      }
    }

    current = (current + 1) % settings->interval;
  }

  free(selection);
  selection = NULL;

  return result;
}

int main(int argc, char *argv[]) {

  settings_t * settings = parse_settings(argc, argv);

  if (settings->init == 1) {
    create_files(settings);
  }

  result_t * result = perform_benchmark(settings);

  printf("Total Data: %lu\n", result->r_bytes);
  printf("Total Operations: %lu\n", result->r_bytes);
  printf("Throughput: %f MB/s | %f IOPS\n", (double) (result->r_bytes) / ((double) result->r_time / 1000.0), ((double) result->r_ops) / ((double) result->r_time / 1000000000.0));
  printf("Total Time: %f\n", (double) result->r_time / 1000000000.0);

  free(result);
  free(settings);

  return 0;
}