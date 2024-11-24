#include "directory.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef DIR directory_t;
typedef struct dirent directory_entry_t;
typedef struct stat stat_t;

void list_files(const char * directory_path, char *** entries_buffer, int * entries_size) {

    int capacity = 4, index = 0;
    char * file;
    char ** files = malloc(capacity * sizeof(char *));

    directory_t * directory = opendir(directory_path);
    directory_entry_t * directory_entry;
    stat_t stat;

    while ((directory_entry = readdir(directory)) != NULL) {

        // allocate more space if nessecary
        if (index == capacity) {
            capacity = capacity * 2;
            files = realloc(files, capacity * sizeof(char *));
        }

        // allocate space for file path
        file = malloc(PATH_MAX * sizeof(char));

        // assemble absolute path of file
        strcpy(file, directory_path);
        strcat(file, "/");
        strcat(file, directory_entry->d_name);

        // stat file to get file mode
        if (lstat(file, &stat) != -1 && !S_ISREG(stat.st_mode)) {
            free(file);
            continue;
        }

        // copy reference to
        files[index] = file;

        // increment counter
        index++;
    }

    // close directory
    closedir(directory);

    // set address of results to output locations
    *entries_buffer = files;
    *entries_size = index;
}
