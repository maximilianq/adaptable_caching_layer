#define _GNU_SOURCE

#include "path.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <linux/limits.h>

#include "../constants.h"

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