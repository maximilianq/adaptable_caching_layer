#include "extent.h"

#include <stdlib.h>
#include <stdio.h>

void init_internal_extent(internal_extent_t * extent) {
    extent->e_root = NULL;
    extent->e_size = 0;
}

void free_internal_extent(internal_extent_t * extent) {

    while (extent->e_root != NULL) {
        extent_entry_t * entry = extent->e_root;
        extent->e_root = entry->ee_next;
        printf("extent %li %li dropped!\n", entry->ee_lower, entry->ee_upper);
        free(entry);
    }

    extent->e_size = 0;
}

void push_internal_extent(internal_extent_t * extent, ssize_t lower, ssize_t upper) {
    if (extent->e_root == NULL) {
        extent->e_root = malloc(sizeof(extent_entry_t));
        extent->e_root->ee_lower = lower;
        extent->e_root->ee_upper = upper;
        extent->e_root->ee_next = NULL;
        extent->e_size++;
        return;
    }

    extent_entry_t *current = extent->e_root;
    extent_entry_t *prev = NULL;

    while (current != NULL && current->ee_upper < lower) {
        prev = current;
        current = current->ee_next;
    }

    if (current != NULL && current->ee_lower <= upper) {

        current->ee_lower = min(current->ee_lower, lower);
        current->ee_upper = max(current->ee_upper, upper);

        while (current->ee_next != NULL && current->ee_next->ee_lower <= current->ee_upper) {
            extent_entry_t *next = current->ee_next;
            current->ee_upper = max(current->ee_upper, next->ee_upper);
            current->ee_next = next->ee_next;
            free(next);
            extent->e_size--;
        }
        return;
    }

    extent_entry_t *new_entry = malloc(sizeof(extent_entry_t));
    new_entry->ee_lower = lower;
    new_entry->ee_upper = upper;
    new_entry->ee_next = current;

    if (prev == NULL) {
        extent->e_root = new_entry;
    } else {
        prev->ee_next = new_entry;
    }

    extent->e_size++;
}

int pop_internal_extent(internal_extent_t * extent, ssize_t * lower, ssize_t * upper) {

    if (extent->e_root == NULL) {
        return -1;
    }

    extent_entry_t * entry = extent->e_root;
    extent->e_root = entry->ee_next;

    *lower = entry->ee_lower;
    *upper = entry->ee_upper;

    free(entry);

    extent->e_size--;

    return 0;
}