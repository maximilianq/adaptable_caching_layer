#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <linux/limits.h>

#include "../library/external/acl.h"

#define PATH SOURCE_PATH
#define FILE "custom/file_%d.data"

typedef struct result {
  ssize_t r_bytes;
  long r_ops;
} result_t;

void init(int files, long filesize) {

  char path_template[PATH_MAX];
  snprintf(path_template, PATH_MAX, "%s/%s", PATH, FILE);

  srand(time(NULL));

  char * buffer = malloc(filesize * sizeof(char));
  for (int i = 0; i < filesize; i++) {
    buffer[i] = (char) (rand() % 26) + 65;
  }

  for (int i = 0; i < files; i++) {

    char path[PATH_MAX];
    snprintf(path, PATH_MAX, path_template, i);

    int file = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (file == -1) {
      perror("Error: could not create benchmark file.");
      exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(file, buffer, filesize);
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

int * compute(int files, int patterns, int seed) {

  int * selection = malloc(sizeof(int) * patterns);
  if (selection == NULL) {
    perror("Error: could not allocate selection.");
    exit(EXIT_FAILURE);
  }

  srand(seed);

  for (int i = 0; i < patterns; i++) {
    selection[i] = rand() % files;
  }

  return selection;
}

result_t * benchmark(const int runtime, const int files, const int window, const int iterations, const int blocksize, const int patterns, const int filesize, const int seed) {

  // allocate result structure
  result_t * result = malloc(sizeof(result_t));
  result->r_bytes = 0;
  result->r_ops = 0;

  // create file path template to be utilized as
  char path_template[PATH_MAX];
  snprintf(path_template, PATH_MAX, "%s/%s", PATH, FILE);

  // Allocate aligned buffer for reading
  void * buffer;
  if (posix_memalign(&buffer, 4096, blocksize) != 0) {
    perror("Error allocating aligned memory\n");
    exit(EXIT_FAILURE);
  }

  // compute access pattern
  int * selection = compute(files, patterns, seed);

  // seed random number generator
  srand(seed);

  // initialize pointer to files
  int current = 0, prefetch = window - 1;

  // init start time to be utilized in limiting the application runtime
  time_t start_time = time(NULL);
  while (time(NULL) < start_time + runtime) {

    // fill up the prefetch queue according to the prefetch window
    while (prefetch != (current + window) % patterns) {

      // build file path given the current index
      char path[PATH_MAX];
      snprintf(path, PATH_MAX, path_template, selection[prefetch]);

      acl_advise(path, ACL_WILLNEED);

      prefetch = (prefetch + 1) % patterns;
    }

    char path[PATH_MAX];
    snprintf(path, PATH_MAX, path_template, selection[current]);

    printf("accessing file %s\n", path);

    int file = open(path, O_DIRECT | O_WRONLY);
    if (file == -1) {
      perror("Error: could not open benchmark file.");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < iterations; i++) {

      off_t offset = ((rand() % filesize) / blocksize) * blocksize;

      if (lseek(file, offset, SEEK_SET) == -1) {
        perror("Error: could not seek benchmark file.");
        exit(EXIT_FAILURE);
      }

      ssize_t bytes_write = write(file, buffer, blocksize);
      if (bytes_write == -1) {
        perror("Error: could not read benchmark data.");
        exit(EXIT_FAILURE);
      }

      result->r_bytes += bytes_write;
      result->r_ops += 1;

      usleep(25);
    }

    if (close(file) == -1) {
      perror("Error: could not close benchmark file.");
      exit(EXIT_FAILURE);
    }

    int lookahead = current + 1;
    while (lookahead != (current + 2 * window) % patterns) {

      if (selection[lookahead] == selection[current]) {
        break;
      }

      lookahead = (lookahead + 1) % patterns;
    }

    if (lookahead == (current + 2 * window) % patterns) {

      // build file path given the current index
      char path[PATH_MAX];
      snprintf(path, PATH_MAX, path_template, selection[prefetch]);

      //acl_advise(path, ACL_DONTNEED, 1);
    }

    current = (current + 1) % patterns;
  }

  free(selection);
  selection = NULL;

  return result;
}

int main() {

  //init(32, 67108864);

  result_t * result = benchmark(300, 32, 10, 1024, 4096, 1024, 67108864, 1234567890);

  printf("-----> %lu\n", result->r_bytes);
  printf("-----> %lu\n", result->r_ops);

  free(result);

  return 0;
}