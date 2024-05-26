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
    if ( init_cq(&queue, 64) != 0 )
    {
        printf("init_queue: failed\n");
        return EXIT_FAILURE;
    }

    push_cq(&queue, data, strlen(data)); // 16
    push_cq(&queue, data, strlen(data)); // 32
    push_cq(&queue, data, strlen(data)); // 48
    push_cq(&queue, data, strlen(data)); // 64
    push_cq(&queue, data2, strlen(data2)); // 64
    pop_cq(&queue, buf); // 48
    push_cq(&queue, data2, strlen(data2)); // 64

    printf("End of Program\n");
    fflush(stdout);

    return EXIT_SUCCESS;
}
