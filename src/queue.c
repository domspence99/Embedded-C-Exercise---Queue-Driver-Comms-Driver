#include "queue.h"


//Function to initialise queue
QueueStatus Queue_Init(Queue **q, size_t capacity)
{
    (void)q;
    (void)capacity;
    return QUEUE_OK;
}

//Function to send to queue
QueueStatus Queue_Send(Queue *q, const void *data, size_t size)
{
    (void)q;
    (void)data;
    (void)size;
    return QUEUE_OK;
}

//Function to read queue
QueueStatus Queue_Read(Queue *q, void *out, size_t out_cap, size_t *out_size)
{
    (void)q;
    (void)out;
    (void)out_cap;
    (void)out_size;
    return QUEUE_ERR_EMPTY;
}

//Function to close queue
void Queue_Close(Queue *q)
{
    (void)q;
}
