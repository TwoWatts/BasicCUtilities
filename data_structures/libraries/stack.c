#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

/* Debug Macros */
//#define DEBUG_CQ

/* Local Types */
typedef stack_header_t header_t;

/* Local Enumerations */
enum errors_e
{
    ERR_NULL_INSTANCE = -1,
    ERR_MALLOC_FAIL = -2,
    ERR_TEST_FAIL = -3,
    SUCCESS = 0,
};

/* Local Macros */
#define __CUTILS_SALIGN(x) (((x)+7+sizeof(header_t)) & ~7)

/* Public Function(s) */
int
init_stack (
    stack_t * const ins,
    const header_t size
)
{
    if ( !ins )
        return ERR_NULL_INSTANCE;

    ins->top      = 0;
    ins->capacity = 0;

    if ( !(ins->stack = malloc(size)) )
        return ERR_MALLOC_FAIL;

    ins->capacity = size;
    return SUCCESS;
}

uint32_t
push_stack(
    stack_t * const ins,
    const uint8_t * const data,
    const header_t size
)
{
    header_t payload = __CUTILS_SALIGN(size);

    /*
     * Return if the stack would overflow
     * after adding this entry.
     */
    if ( (ins->top + payload) > ins->capacity )
    {
        return 0;
    }

    /*
     * Push data onto the stack.
     */
    memcpy(&ins->stack[ins->top], data, size);
    ins->top += payload;
    *(header_t*)&ins->stack[ins->top-sizeof(header_t)] = size;
    return 1;
}

uint32_t
pop_stack(
    stack_t * const ins,
    uint8_t * const data,
    const uint32_t limit
)
{
    header_t size;

    /*
     * Return if the data destination
     * can't hold the payload.
     */
    if ( !data || (size > limit) )
    {
        printf("%s: invalid destination\n", __FUNCTION__);
        return 0;
    }

    /*
     * Return if the stack is empty.
     */
    if ( ins->top == 0 )
    {
        return 0;
    }

    /*
     * Pop data off the stack.
     */
    size = *(header_t*)&ins->stack[ins->top-sizeof(header_t)];
    ins->top -= __CUTILS_SALIGN(size);
    memcpy(data, &ins->stack[ins->top], size);
    return size;
}

/* Undefine Local Macros*/
#undef __CUTILS_SALIGN