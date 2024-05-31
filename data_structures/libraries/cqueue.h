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
queue_init (
    cqueue_t * const ins,
    void *buffer,
    const uint32_t size
);

int
queue_free (
    cqueue_t * const ins
);

void
queue_clear (
    cqueue_t * const ins
);

uint32_t
queue_add (
    cqueue_t * const ins,
    const uint8_t * data,
    uint32_t size
);

uint32_t
queue_rem (
    cqueue_t * const ins,
    uint8_t * data
);

#endif /* CUTILS_QUEUE_H */