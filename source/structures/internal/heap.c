#include "heap.h"

#include <stdlib.h>

void swap(internal_heap_t * heap, int first, int second) {

    // swap entrys at the given two entries
    void * entry = heap->h_entries[first];
    heap->h_entries[first] = heap->h_entries[second];
    heap->h_entries[second] = entry;

    // if listen function is defined execute for both nodes
    if (heap->h_listen != NULL) {
        heap->h_listen(first, heap->h_entries[first]->he_score, heap->h_entries[first]->he_data, heap->h_listen_args);
        heap->h_listen(second, heap->h_entries[second]->he_score, heap->h_entries[second]->he_data, heap->h_listen_args);
    }
}

int upheap(internal_heap_t * heap, int index) {

    // swap parent with entry until heap property is statisfied again
    while (index != 0 && heap->h_compare(heap->h_entries[(index - 1) / 2]->he_score, heap->h_entries[index]->he_score) > 0) {

        // calculate parent and swap entries
        int parent = (index - 1) / 2;
        swap(heap, index, parent);

        // set parent as next index to be verified
        index = parent;
    }

    return index;
}

int downheap(internal_heap_t * heap, int index) {

    // calculate left and right index as well as initialize minimum with entry index
    int left = (2 * index) + 1;
    int right = (2 * index) + 2;
    int minimum = index;

    // if left child is smaller then element define as new minimum
    if (left < heap->h_size && heap->h_compare(heap->h_entries[left]->he_score,  heap->h_entries[minimum]->he_score) < 0) {
        minimum = left;
    }

    // if right child is smaller then element or left child set right as new minimum
    if (right < heap->h_size && heap->h_compare(heap->h_entries[right]->he_score, heap->h_entries[minimum]->he_score) < 0) {
        minimum = right;
    }

    // if a minimum except for entry was found swap and continue downheap from that position
    if (minimum != index) {
        swap(heap, index, minimum);
        index = downheap(heap, minimum);
    }

    return index;
}

void init_internal_heap(internal_heap_t * heap, int capacity, compare_t compare, listen_t listen, void * listen_args, insert_t insert, update_t update) {

    // allocate memory for heap entries based on given capacity
    heap->h_entries = malloc(capacity * sizeof(heap_entry_t *));

    // define current size and capacity
    heap->h_size = 0;
    heap->h_capacity = capacity;

    // define compare function
    heap->h_compare = compare;

    // deine listen function and listen args called on entry index change
    heap->h_listen = listen;
    heap->h_listen_args = listen_args;

    heap->h_insert = insert;
    heap->h_update = update;
}

void free_internal_heap(internal_heap_t * heap) {

    // free memory for heap entries
    free(heap->h_entries);
}

int insert_internal_heap(internal_heap_t * heap, unsigned long score, void * data) {

    // calculate new score if required
    if (heap->h_insert != NULL) {
        score = heap->h_insert();
    }

    // allocate and initialize new heap entry
    heap_entry_t * entry = malloc(sizeof(heap_entry_t));
    entry->he_score = score;
    entry->he_data = data;

    // add entry to cache and increase size of heap
    heap->h_entries[heap->h_size] = entry;
    heap->h_size = heap->h_size + 1;

    // upheap until heap property statisfied
    int index = upheap(heap, heap->h_size - 1);

    // return current index of entry
    return index;
}

void * remove_internal_heap(internal_heap_t * heap, int index) {

    if (index >= heap->h_size) {
        return NULL;
    }

    // retrieve entry at specified index and save data of entry
    heap_entry_t * entry = heap->h_entries[index];
    void * result = entry->he_data;

    heap->h_size = heap->h_size - 1;

    // swap with last entry and reduce heap size by one
    swap(heap, index, heap->h_size);

    // downheap if item is larger then parent or if already root otherwise upheap
    if (index == 0 || heap->h_compare(heap->h_entries[index / 2]->he_score, heap->h_entries[index]->he_score) < 0) {

        // check all entries below removed ones
        downheap(heap, index);

        // free key and entry
        free(entry);

        // return removed entry
        return result;
    }

    // check all entries above removed ones
    upheap(heap, index);

    // free key and entry
    free(entry);

    // return removed entry
    return result;
}

int update_internal_heap(internal_heap_t * heap, int index, unsigned long score) {

    // if score is increased check all below
    if (index == 0 || heap->h_compare(heap->h_entries[index]->he_score, score) < 0) {

        // calculate new score if required
        unsigned long prev = heap->h_entries[index]->he_score;
        if (heap->h_update != NULL) {
            score = heap->h_update(prev);
        }

        // update score and update entries below
        heap->h_entries[index]->he_score = score;
        int result = downheap(heap, index);

        return result;
    }

    // update score and update entries above
    heap->h_entries[index]->he_score = score;
    int result = upheap(heap, index);

    return result;
}