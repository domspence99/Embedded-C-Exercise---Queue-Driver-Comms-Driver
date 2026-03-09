#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "comm.h"



int main(void)
{
    Queue *q = NULL;
    Queue_Init(&q, 24);

    char buffer[32];
    size_t received;

    Queue_Send(q, "AAAAA", 5);
    Queue_Send(q, "BBBB", 4);

    Queue_Read(q, buffer, sizeof(buffer), &received);
    buffer[received] = '\0';
    printf("Read: %s\n", buffer);

    Queue_Send(q, "CCCCCCCC", 8);

    Queue_Read(q, buffer, sizeof(buffer), &received);
    buffer[received] = '\0';
    printf("Read: %s\n", buffer);

    Queue_Read(q, buffer, sizeof(buffer), &received);
    buffer[received] = '\0';
    printf("Read: %s\n", buffer);

    Queue_Close(q);
}

