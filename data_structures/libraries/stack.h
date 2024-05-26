#ifndef CUTILS_STACK_H
#define CUTILS_STACK_H

#include <stdint.h>
typedef uint32_t stack_header_t;

typedef struct
{
    uint8_t *stack;
    stack_header_t top;
    stack_header_t capacity;
} stack_t;

int
init_stack (
    stack_t * const ins,
    const stack_header_t size
);

uint32_t
push_stack(
    stack_t * const ins,
    const uint8_t * const data,
    const stack_header_t size
);

uint32_t
pop_stack(
    stack_t * const ins,
    uint8_t * const data,
    const stack_header_t limit
);

#endif /* CUTILS_STACK_H */