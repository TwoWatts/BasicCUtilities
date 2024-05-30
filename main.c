#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cqueue.h"

int main()
{
    int ret;
    char data[] = "testdatx"; // 8 bytes
    char data2[] = "moredatx";
    char buf[120+1];
    memset(buf, ' ', sizeof(buf));
    buf[sizeof(buf)-1] = 0;

    printf("Start of Program\n");

    cqueue_t queue;
    cqueue_t *ins = &queue;
    if ( init_cq(ins, 64) != 0 )
    {
        printf("init_queue: failed\n");
        return EXIT_FAILURE;
    }

    push_cq(ins, data, strlen(data)); // 16
    push_cq(ins, data, strlen(data)); // 32
    push_cq(ins, data, strlen(data)); // 48
    push_cq(ins, data, strlen(data)); // 64
    push_cq(ins, data2, strlen(data2)); // 64
    pop_cq(ins, buf); // 48
    pop_cq(ins, buf); // 32
    pop_cq(ins, buf); // 16
    pop_cq(ins, buf); // 0
    pop_cq(ins, buf); // 0
    push_cq(ins, data2, strlen(data2)); // 16
    pop_cq(ins, buf); // 0

    printf("End of Program\n");
    fflush(stdout);

    return EXIT_SUCCESS;
}
