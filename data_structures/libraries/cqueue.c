#include "cqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

/* Debug Macros */
#define DEBUG_CQ

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
#define __CUTILS_8ALIGN(x) (((x)+7) & ~7)
#define HEADER_SIZE (__CUTILS_8ALIGN(sizeof(header_t)))
#define __CUTILS_QALIGN(x) (__CUTILS_8ALIGN(x+HEADER_SIZE))
#define show_status(x, y) queue_status(x, __FUNCTION__, y)
#define error_out(x) printf("%s: " x "\n", __FUNCTION__)

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
    uint32_t adj_size = __CUTILS_8ALIGN(size);

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
    uint32_t payload = __CUTILS_QALIGN(size);
    uint32_t writer_new = ins->writer + payload;

    #ifdef DEBUG_CQ
    char debug0[] = "none";
    char debug1[] = "wrap-around";
    char debug2[] = "no-wrap-around";
    char *debug_ptr = debug0;
    #endif

    /*
     * Return if the queue would overflow
     * after adding this entry.
     */
    if ( (ins->lsize + payload) > ins->capacity )
    {
        error_out("queue is full... dropping data");
        return FAILURE;
    }

    if ( writer_new <= ins->capacity )
    {
        /*
         * If the payload fits without wrapping
         * around the buffer.
         */
        *(header_t*)&ins->queue[ins->writer] = size;
        ins->writer += HEADER_SIZE;
        memcpy(&ins->queue[ins->writer], data, size);
        ins->writer = ( writer_new != ins->capacity ) ? ( writer_new ) : ( 0 );
        #ifdef DEBUG_CQ
        debug_ptr = debug1;
        #endif
    }
    else
    {
        /*
         * If the payload needs to wrap-around the buffer,
         * the push operation requires 2 memcpy calls.
         * Eq. payload = len(seg1) + len(seg2)
         */
        header_t seg1 = ins->capacity - ins->writer;
        header_t seg2 = writer_new - ins->capacity;

        /* Handle Segment 1 */
        if ( seg1 > HEADER_SIZE )
        {
            /* Segment 1 has a header and data. */
            *(header_t*)&ins->queue[ins->writer] = size;
            ins->writer += HEADER_SIZE;
            seg1 -= HEADER_SIZE;
            memcpy(&ins->queue[ins->writer], data, seg1);
            data += seg1;
            ins->writer = 0;
        }
        else if ( seg1 == HEADER_SIZE )
        {
            /* Segment 1 has a header */
            *(header_t*)&ins->queue[ins->writer] = size;
            ins->writer = 0;
        }
        else
        {
            /* Segment 1 has nothing. */
            if ( seg1 != 0 )
            {
                error_out("queue misalignment is causing corruption");
                return ERR_MISALIGNMENT;
            }
            seg2 -= HEADER_SIZE;
            *(header_t*)&ins->queue[0] = size;
            ins->writer = HEADER_SIZE;
        }

        /* Handle Segment 2 */
        memcpy(&ins->queue[ins->writer], data, seg2);
        ins->writer = seg2;
        #ifdef DEBUG_CQ
        debug_ptr = debug2;
        #endif
    }

    /* Increase "logical size" of queue */
    ins->lsize += payload;

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
    header_t seg1;

    #ifdef DEBUG_CQ
    char debug0[] = "none";
    char debug1[] = "reader == capacity";
    char debug2[] = "seg1 >";
    char debug3[] = "seg1 ==";
    char *debug_ptr = debug0;
    #endif

    /*
     * Return if the queue is empty.
     */
    if ( ins->lsize == 0 )
    {
        error_out("queue is empty");
        return SUCCESS;
    }

    /*
     * Return if the data destination
     * can't hold the payload.
     */
    if ( !data )
    {
        error_out("invalid destination");
        return FAILURE;
    }

    /*
     * Find entry size.
     */

    if ( ins->reader < ins->capacity )
    {
        seg1 = ins->capacity - ins->reader;
        if ( seg1 > HEADER_SIZE )
        {
            size = *(header_t*)&ins->queue[ins->reader];
            payload = __CUTILS_QALIGN(size);
            if ( (ins->reader + size) <= ins->capacity )
            {
                memcpy(data, &ins->queue[ins->reader+HEADER_SIZE], size);
                ins->reader += payload;
            }
            else
            {
                seg1 -= HEADER_SIZE;
                memcpy(data, &ins->queue[ins->reader+HEADER_SIZE], seg1);
                data += seg1;
                memcpy(data, &ins->queue[0], size-seg1);
                ins->reader = __CUTILS_8ALIGN(payload-seg1);
            }
            #ifdef DEBUG_CQ
            debug_ptr = debug2;
            #endif
        }
        else if ( seg1 == HEADER_SIZE )
        {
            size = *(header_t*)&ins->queue[ins->reader];
            memcpy(data, &ins->queue[0], size);
            ins->reader = __CUTILS_8ALIGN(size);
            #ifdef DEBUG_CQ
            debug_ptr = debug3;
            #endif
        }
        else
        {
            error_out("queue misalignment is causing corruption");
            return ERR_MISALIGNMENT;
        }
    }
    else if ( ins->reader == ins->capacity )
    {
        size = *(header_t*)&ins->queue[0];
        memcpy(data, &ins->queue[HEADER_SIZE], size);
        ins->reader = __CUTILS_8ALIGN(size);
        #ifdef DEBUG_CQ
        debug_ptr = debug1;
        #endif
    }
    else
    {
        error_out("reader out of bounds");
        return FAILURE;
    }

    /* Decrease "logical size" of queue */
    ins->lsize -= payload;

    #ifdef DEBUG_CQ
    show_status(ins, debug_ptr);
    #endif

    return size;
}

/* Undefine Local Macros*/
#undef HEADER_SIZE
#undef __CUTILS_8ALIGN
#undef __CUTILS_QALIGN
#undef show_status
#undef error_out
