#include "markov.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lookup.h"

void init_internal_markov(internal_markov_t * markov, int capacity) {

  // allocate space for label list
  markov->m_labels = malloc(capacity * sizeof(char *));
  for (int i = 0; i < capacity; i++) markov->m_labels[i] = NULL;

  // allocate space for adjacency matrix
  markov->m_values = malloc(capacity * capacity * sizeof(int));
  for (int i = 0; i < capacity * capacity; i++) markov->m_values[i] = 0;

  // initialize size and capacity of markov chain
  markov->m_size = 0;
  markov->m_capacity = capacity;

  // set previous to -1 indicating that no previous element was updated yet
  markov->m_previous = -1;
}

void free_internal_markov(internal_markov_t * markov) {
  free(markov->m_labels);
  free(markov->m_values);
}

void resize_internal_markov(internal_markov_t * markov, int capacity) {

  // allocate new space for label list
  char ** labels = malloc(capacity * sizeof(char *));
  for (int i = 0; i < capacity; i++)
    labels[i] = NULL;
  memcpy(labels, markov->m_labels, markov->m_capacity * sizeof(char *));

  // allocate new space for extended adjacency matrix
  int * values = malloc(capacity * capacity * sizeof(int));
  for (int i = 0; i < capacity * capacity; i++)
    values[i] = 0;
  for (int i = 0; i < markov->m_capacity; i++)
    memcpy(&values[capacity * i], &markov->m_values[markov->m_capacity * i], markov->m_capacity * sizeof(int));

  // free old adjacency matrix
  free(markov->m_labels);
  free(markov->m_values);

  // replace adjacency matrices and capacity
  markov->m_labels = labels;
  markov->m_values = values;
  markov->m_capacity = capacity;
}

void update_internal_markov(internal_markov_t * markov, char * path) {

  int index = -1;
  for (int i = 0; i < markov->m_size; i++) {
    if (strcmp(markov->m_labels[i], path) == 0) {
      index = i;
    }
  }

  // search index of associated label
  if (index == -1) {

    // if size reaches maximum capacity resize markov chain
    if (markov->m_size == markov->m_capacity) resize_internal_markov(markov, markov->m_capacity * 2);

    // set index to current size and increase size
    index = markov->m_size;
    markov->m_size += 1;

    // insert path and index to lookup table
    markov->m_labels[index] = strdup(path);
  }

  // if no previous element was defined, set previous to current index and return without any modification
  if (markov->m_previous == -1) {
    markov->m_previous = index;
    return;
  }

  markov->m_values[(markov->m_previous * markov->m_capacity) + index] += 1;

  markov->m_previous = index;
}

char * retrieve_internal_markov(internal_markov_t * markov, char * path) {

  // search index of associated label
  int index = -1;
  for (int i = 0; i < markov->m_size; i++) {
    if (strcmp(path, markov->m_labels[i]) == 0) {
      index = i;
      break;
    }
  }

  // if no entry could be found in markov chain return NULL
  if (index == -1) {
    return NULL;
  }

  // search maximum in neighboring list
  int max = 0;
  for (int i = 0; i < markov->m_size; i++) {
    if (markov->m_values[(index * markov->m_capacity) + i] > markov->m_values[(index * markov->m_capacity) + max]) {
      max = i;
    }
  }

  // return label of associated maximum
  return strdup(markov->m_labels[max]);
}