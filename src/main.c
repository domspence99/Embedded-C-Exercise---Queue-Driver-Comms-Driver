#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "comm.h"


int main(void)
{
    //Test for queue initialisation
    Queue *q = NULL;
    QueueStatus st = Queue_Init(&q, 128);

    if (st == QUEUE_OK){
        printf("Queue initialized\n");
    }
    else {
        printf("Error: %d",st);
    }

    //Closes queue, frees memory, prevents leaks
    Queue_Close(q);
    printf("Queue closed");
    
    return 0;
}