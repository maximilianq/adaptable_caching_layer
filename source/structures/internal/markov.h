#ifndef INTERNAL_MARKOV_H
#define INTERNAL_MARKOV_H


struct internal_markov {

    char ** m_labels;
    int * m_values;

    int m_size;
    int m_capacity;

    int m_previous;
};

typedef struct internal_markov internal_markov_t;

void init_internal_markov(internal_markov_t * markov, int capacity);

void free_internal_markov(internal_markov_t * markov);

void resize_internal_markov(internal_markov_t * markov, int capacity);

void update_internal_markov(internal_markov_t * markov, char * path);

char * retrieve_internal_markov(internal_markov_t * markov, char * path);

#endif
