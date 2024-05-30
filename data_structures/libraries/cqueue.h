#ifndef CUTILS_QUEUE_H
#define CUTILS_QUEUE_H

/*
AUTHOR: TWOWATTS
*/

#include <stdint.h>

typedef struct
{
    uint8_t *queue;    // Queue Buffer
    uint32_t writer;   // Queue Write Index
    uint32_t reader;   // Queue Read Index
    uint32_t capacity; // Queue Capacity
    uint32_t lsize;    // Logical Queue Size
    uint8_t flags;
} cqueue_t;

int
init_cq (
    cqueue_t * const ins,
    void *buffer,
    const uint32_t size
);

int
free_cq (
    cqueue_t * const ins
);

void
clear_cq (
    cqueue_t * const ins
);

uint32_t
add_cq (
    cqueue_t * const ins,
    const uint8_t * data,
    uint32_t size
);

uint32_t
rem_cq (
    cqueue_t * const ins,
    uint8_t * data
);

#endif /* CUTILS_QUEUE_H */