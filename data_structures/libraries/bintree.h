#ifndef CUTILS_BINTREE_H
#define CUTILS_BINTREE_H

#include <stdio.h>

#define ERROR_WRAP(r, c, y) if ( (r = y) != c ) printf(#y " failed: error code %d\n", r)

// Types
typedef unsigned int bintree_count_t;

typedef unsigned long long bintree_value_t;

typedef unsigned int bintree_index_t;

typedef union 
{
    struct
    {
        unsigned long long value : 63;
        unsigned long long valid : 1;
    } field;
    unsigned long long v;
} bintree_key_t;

typedef struct bintree_entry_s
{
    char bf;
    bintree_key_t key;
    bintree_value_t value;
    struct bintree_entry_s *left;
    struct bintree_entry_s *right;
} bintree_entry_t;

typedef struct
{
    bintree_count_t mem_alloc;
    bintree_count_t layers;
    bintree_count_t entry_count;
    bintree_count_t max_entry_count;
    bintree_entry_t *data;
    bintree_entry_t *root;
} bintree_t;

// Function Prototypes
void bintree_clear_entries();
void bintree_free();
void bintree_print();
void bintree_visualize();
int bintree_init();
bintree_entry_t* bintree_search();
int bintree_insert();
int bintree_remove();

#endif /* CUTILS_BINTREE_H */