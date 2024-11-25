#include "path.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <linux/limits.h>

char * source_location = NULL;
char * cache_location = NULL;

char * get_full_path(const char * input, char * output) {

    if (input[0] == '/') {
        strcpy(output, input);
        return output;
    }

    getcwd(output, PATH_MAX);
    strcat(output, "/");
    strcat(output, input);

    return output;
}

char * get_cache_path(const char * input, char * output) {

    if (source_location == NULL) {

        const char * source_location_string = getenv("ACL_SOURCE_PATH");
        if (source_location_string == NULL) {
            perror("Error: could not retrieve source path!");
            exit(EXIT_FAILURE);
        }

        source_location = malloc(PATH_MAX);
        strcpy(source_location, source_location_string);
    }

    if (cache_location == NULL) {

        const char * cache_location_string = getenv("ACL_CACHE_PATH");
        if (cache_location_string == NULL) {
            perror("Error: could not retrieve cache path!");
            exit(EXIT_FAILURE);
        }

        cache_location = malloc(PATH_MAX);
        strcpy(cache_location, cache_location_string);
    }

    char source_path[PATH_MAX];
    if (input[0] != '/') {
        get_full_path(input, source_path);
    } else {
        strcpy(source_path, input);
    }

    if (strncmp(source_location, source_path, strlen(source_location)) != 0) {
        return NULL;
    }

    strcpy(output, cache_location);
    strcat(output, source_path + strlen(source_location));

    size_t index = strlen(cache_location) + 1;
    while (index < PATH_MAX && output[index] != '\0') {
        if (output[index] == '/') {
            output[index] = '_';
        }
        index++;
    }

    return output;
}

int is_cacheable_path(const char * path) {

    if (source_location == NULL) {

        const char * source_location_string = getenv("ACL_SOURCE_PATH");
        if (source_location_string == NULL) {
            perror("Error: could not retrieve source path!");
            exit(EXIT_FAILURE);
        }

        source_location = malloc(PATH_MAX);
        strcpy(source_location, source_location_string);
    }

    return strncmp(source_location, path, strlen(source_location)) == 0;
}

char * get_parent_path(const char * input, char * output) {

    strcpy(output, input);

    for (int i = strlen(output); i >= 0; i--) {
        if (output[i] == '/') {
            output[i] = '\0';
            break;
        }
    }

    return output;
}

int compare_path(const void * a, const void * b) {

    const char * str1 = *(const char **) a;
    const char * str2 = *(const char **) b;

    while (*str1 && *str2) {

        // if current item is a digit compare numbers
        if (isdigit(*str1) && isdigit(*str2)) {

            char *end1, *end2;

            long num1 = strtol(str1, &end1, 10);
            long num2 = strtol(str2, &end2, 10);

            if (num1 != num2) {
                return (num1 > num2) - (num1 < num2);
            }

            // If numbers are the same, advance both pointers
            str1 = end1;
            str2 = end2;
        }

        // compare non numeric characters
        else {

            if (*str1 != *str2) {
                return (unsigned char) *str1 - (unsigned char) *str2;
            }

            str1++;
            str2++;
        }
    }

    return (unsigned char)*str1 - (unsigned char)*str2;
}