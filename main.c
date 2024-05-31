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
    if ( queue_init(ins, NULL, 64) != 0 )
    {
        printf("init_queue: failed\n");
        return EXIT_FAILURE;
    }

    queue_add(ins, data, strlen(data)); // 16
    queue_add(ins, data, strlen(data)); // 32
    queue_add(ins, data, strlen(data)); // 48
    queue_add(ins, data, strlen(data)); // 64 - Fail
    queue_add(ins, data2, strlen(data2)); // 64 - Fail
    queue_rem(ins, buf); // 32
    queue_rem(ins, buf); // 16
    queue_rem(ins, buf); // 0
    queue_rem(ins, buf); // 0
    queue_rem(ins, buf); // 0
    queue_add(ins, data2, strlen(data2)+1); // 16
    queue_rem(ins, buf); // 0
    printf("%s", buf);

    queue_free(ins);
    printf("End of Program\n");
    fflush(stdout);

    return EXIT_SUCCESS;
}
