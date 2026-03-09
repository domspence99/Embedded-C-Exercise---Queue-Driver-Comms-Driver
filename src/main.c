#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "comm.h"



int main(void)
{
    //Test for queue initialisation
    Queue *q = NULL;
    Queue_Init(&q, 24);

    //Test queueSend
    Queue_Send(q, "12345", 5); //(7 total bytes)
    Queue_Send(q, "6789", 4); //(6 total bytes)
    Queue_Send(q, "ABCDE", 5); //(7 total bytes)


    //Closes queue, frees memory, prevents leaks
    Queue_Close(q);
    printf("Queue closed");
    
    return 0;
}

