#include "cqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

/* Debug Macros */
//#define DEBUG_CQ

/* Local Types */
typedef uint64_t header_t;

/* Local Enumerations */
enum errors_e
{
    FAILURE = -1,
    ERR_NULL_INSTANCE = -2,
    ERR_MALLOC_FAIL = -3,
    ERR_TEST_FAIL = -4,
    ERR_QUEUE_FULL = -5,
    ERR_MISALIGNMENT = -6,
    SUCCESS = 0,
};

/* Private Prototypes */
static void queue_status();

/* Local Macros */
#define __CUTILS_XALIGN(x) (((x)+7) & ~7)
#define HEADER_SIZE (__CUTILS_XALIGN(sizeof(header_t)))
#define __CUTILS_QALIGN(x) (__CUTILS_XALIGN(x+HEADER_SIZE))
#define show_status(x, y) queue_status(x, __FUNCTION__, y)
#define error_out(x) printf("%s: " x "\n", __FUNCTION__)
#define ASSUME(x) if (__builtin_expect((x), 1))
#define DONT_ASSUME(x) if (__builtin_expect((x), 0))
#define RECOVER else

/* Private Function(s) */
static void queue_status(
    cqueue_t * ins,
    const char *function_name,
    char *message
)
{
    printf(
        "%s::%s\n"
        "\tinstance:queue=%p\n"
        "\tinstance:writer=%u\n"
        "\tinstance:reader=%u\n"
        "\tinstance:capacity=%u\n"
        "\tinstance:lsize=%u\n",
        function_name, message,
        ins->queue,
        ins->writer,
        ins->reader,
        ins->capacity,
        ins->lsize);
    printf("\tinstance:<contents>=\"");
    for ( int i = 0; i < ins->capacity; ++i )
        if ( ins->queue[i] >= 32 && ins->queue[i] <= 127 )
            printf("%c", ins->queue[i]);
        else
            printf("_");
    printf("\"\n");
    fflush(stdout);
}

/* Public Function(s) */
int
init_cq (
    cqueue_t * const ins,
    const uint32_t size
)
{
    uint32_t adj_size = __CUTILS_XALIGN(size);

    if ( !ins )
        return ERR_NULL_INSTANCE;

    ins->writer   = 0;
    ins->reader   = 0;
    ins->capacity = 0;
    ins->lsize    = 0;
    
    if ( !(ins->queue = malloc(adj_size)) )
        return ERR_MALLOC_FAIL;

    ins->capacity = adj_size;
    memset(&ins->queue[0], 0, adj_size);

    return SUCCESS;
}

int
free_cq (
    cqueue_t * const ins
)
{
    ins->writer   = 0;
    ins->reader   = 0;
    ins->capacity = 0;
    ins->lsize    = 0;
    free(ins->queue);
    return SUCCESS;
}

void
clear_cq (
    cqueue_t * const ins
)
{
    ins->writer = 0;
    ins->reader = 0;
    ins->lsize  = 0;
    memset(ins->queue, 0, ins->capacity);
}

uint32_t
push_cq (
    cqueue_t * const ins,
    const uint8_t * data,
    uint32_t size
)
{
    header_t seg1, seg2;
    uint32_t payload = __CUTILS_QALIGN(size);
    uint32_t writer_new = ins->writer + payload;

    #ifdef DEBUG_CQ
    char debug0[] = "none";
    char debug1[] = "wrap-around";
    char debug2[] = "no-wrap-around";
    char *debug_ptr = debug0;
    #endif

    /*
     * Return if the data being pushed 
     * is invalid.
     */
    DONT_ASSUME ( !data )
    {
        error_out("data being pushed is invalid");
        return FAILURE;
    }

    /*
     * Return if the queue would overflow
     * after adding this entry.
     */
    if ( (ins->lsize + payload) > ins->capacity )
    {
        error_out("queue is full... dropping data");
        return FAILURE;
    }

    ASSUME ( writer_new <= ins->capacity )
    {
        /*
         * If the payload fits without wrapping
         * around the buffer.
         */
        *(header_t*)&ins->queue[ins->writer] = size;
        memcpy(&ins->queue[ins->writer+HEADER_SIZE], data, size);
        ins->writer = writer_new;
        #ifdef DEBUG_CQ
        debug_ptr = debug1;
        #endif
    }
    RECOVER
    {
        /*
         * If the payload needs to wrap-around the buffer,
         * the push operation typically requires 2 memcpy calls.
         * Eq. payload = len(seg1) + len(seg2)
         */
        seg1 = ins->capacity - ins->writer;
        seg2 = writer_new - ins->capacity;

        if ( seg1 > HEADER_SIZE )
        {
            /* Segment 1 = header + data */
            *(header_t*)&ins->queue[ins->writer] = size;
            seg1 -= HEADER_SIZE;
            memcpy(&ins->queue[ins->writer+HEADER_SIZE], data, seg1);
            memcpy(&ins->queue[0], data+seg1, seg2);
            ins->writer = seg2;
        }
        else if ( seg1 == HEADER_SIZE )
        {
            /* Segment 1 = header */
            *(header_t*)&ins->queue[ins->writer] = size;
            memcpy(&ins->queue[0], data, seg2-HEADER_SIZE);
            ins->writer = seg2;
        }
        else
        {
            /* Segment 1 = nothing */
            *(header_t*)&ins->queue[0] = size;
            memcpy(&ins->queue[ins->writer+HEADER_SIZE], data, seg2-HEADER_SIZE);
            ins->writer = seg2;
        }

        #ifdef DEBUG_CQ
        debug_ptr = debug2;
        #endif
    }

    /* Increase "logical size" of queue */
    ins->lsize += payload;

    /* Wrap writer around buffer. */
    DONT_ASSUME ( ins->writer >= ins->capacity )
        ins->writer = 0;

    #ifdef DEBUG_CQ
    show_status(ins, debug_ptr);
    #endif

    return SUCCESS;
}

uint32_t
pop_cq (
    cqueue_t * const ins,
    uint8_t * data
)
{
    header_t size;
    header_t payload;
    header_t seg1, seg2;
    header_t reader_new;

    #ifdef DEBUG_CQ
    char debug0[] = "none";
    char debug1[] = "reader <= capacity";
    char debug2[] = "reader > capacity";
    char *debug_ptr = debug0;
    #endif

    /*
     * Return if the data destination
     * can't hold the payload.
     */
    DONT_ASSUME ( !data )
    {
        error_out("invalid destination");
        return FAILURE;
    }

    /*
     * Return if the queue is empty.
     */
    if ( ins->lsize <= HEADER_SIZE )
    {
        error_out("queue is empty");
        return SUCCESS;
    }

    size = *(header_t*)&ins->queue[ins->reader];
    payload = __CUTILS_QALIGN(size);
    reader_new = (ins->reader + payload);

    /*
     * Find entry size.
     */
    ASSUME ( reader_new <= ins->capacity )
    {
        memcpy(data, &ins->queue[ins->reader+HEADER_SIZE], size);
        ins->reader = reader_new;
        #ifdef DEBUG_CQ
        debug_ptr = debug1;
        #endif
    }
    RECOVER
    {
        seg1 = ins->capacity - ins->reader;
        seg2 = reader_new - ins->capacity;
        if ( seg1 > HEADER_SIZE )
        {
            /* Segment 1 = header + data */
            seg1 -= HEADER_SIZE;
            memcpy(data, &ins->queue[ins->reader+HEADER_SIZE], seg1);
            memcpy(data+seg1, &ins->queue[0], seg2);
            ins->reader = seg2;
        }
        else if ( seg1 == HEADER_SIZE )
        {
            /* Segment 1 = header */
            memcpy(data, &ins->queue[0], seg2);
            ins->reader = seg2;
        }
        else
        {
            /* Segment 1 = data */
            memcpy(data, &ins->queue[HEADER_SIZE], seg2-HEADER_SIZE);
            ins->reader = seg2;
        }
        #ifdef DEBUG_CQ
        debug_ptr = debug2;
        #endif
    }

    /* Decrease "logical size" of queue */
    ins->lsize -= payload;

    /* Wrap reader around buffer. */
    DONT_ASSUME ( ins->reader >= ins->capacity )
        ins->reader = 0;

    #ifdef DEBUG_CQ
    show_status(ins, debug_ptr);
    #endif

    return size;
}

/* Undefine Local Macros*/
#undef HEADER_SIZE
#undef __CUTILS_XALIGN
#undef __CUTILS_QALIGN
#undef show_status
#undef error_out
#undef ASSUME
#undef DONT_ASSUME
#undef RECOVER
