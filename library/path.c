#define _GNU_SOURCE

#include "path.h"

#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "constants.h"

char * get_full_path(const char * input, char * output, size_t size) {

    if (input[0] == '/') {
        strcpy(output, input);
        return output;
    }

    getcwd(output, size);
    strcat(output, "/");
    strcat(output, input);

    return output;
}

char * get_cache_path(const char * input, char * output, size_t size) {

    char source_path[PATH_MAX];
    if (input[0] != '/') {
        get_full_path(input, source_path, size);
    } else {
        strcpy(source_path, input);
    }

    if (strncmp(SOURCE_PATH, source_path, strlen(SOURCE_PATH)) != 0) {
        return NULL;
    }

    strcpy(output, CACHE_PATH);
    strcat(output, source_path + strlen(SOURCE_PATH));

    int index = strlen(CACHE_PATH) + 1;
    while (index < PATH_MAX && output[index] != '\0') {
        if (output[index] == '/') {
            output[index] = '_';
        }
        index++;
    }

    return output;
}

char * get_parent_path(const char * input, char * output, size_t size) {

    strcpy(output, input);

    for (int i = strlen(output); i >= 0; i--) {
        if (output[i] == '/') {
            output[i] = '\0';
            break;
        }
    }

    return output;
}
