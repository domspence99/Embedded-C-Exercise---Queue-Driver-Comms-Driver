#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

struct Queue
{
    uint8_t *buffer; // allocated memory
    size_t capacity; // size of buffer
    size_t head;     // read pos
    size_t tail;     // write pos
    size_t count;    // no. of bytes stored
};

// Function to initialise queue
QueueStatus Queue_Init(Queue **q, size_t capacity)
{
    // 1. Validate arguments
    // Check that the address of the q object is not a NULL pointer & that there is a capacitity param 
    if (q == NULL || capacity == 0)
    {
        return QUEUE_ERR_INVALID_ARG;
    }
    // 2. Allocate memory for queue structure
    Queue *new_queue = malloc(sizeof(Queue)); // Create a new que an allocate memory

    // 3. Check if memory was allocated
    if (new_queue == NULL)
    {
        return QUEUE_ERR_NO_MEMORY;
    }

    // 4. Allocate memory for buffer of size:capacity
    new_queue->buffer = malloc(capacity);
    // 4.1 Check if memory was allocated for buffer
    if (new_queue->buffer == NULL)
    {
        free(new_queue); // If malloc fails for buffer, clean up previos malloc
        return QUEUE_ERR_NO_MEMORY;
    }

    // 5. Initialise the queue fields
    new_queue->capacity = capacity; // set capacity
    new_queue->head = 0;
    new_queue->tail = 0;
    new_queue->count = 0;

    // 6. Assign new created queue in memory
    *q = new_queue;

    /*
    printf("Queue struct size: %zu\n", sizeof(Queue));
    printf("Queue capacity: %zu\n", capacity);
    printf("Queue struct addr: %p\n", new_queue);
    printf("Buffer addr: %p\n", new_queue->buffer);
    printf("New queue capacity: %zu\n", new_queue->capacity);
    printf("New queue head: %zu\n", new_queue->head);
    printf("New queue head: %zu\n", new_queue->tail);
    printf("New queue head: %zu\n", new_queue->count);
    */

    return QUEUE_OK;
}

// Function to send to queue
QueueStatus Queue_Send(Queue *q, const void *data, size_t size)
{
    (void)q;
    (void)data;
    (void)size;
    return QUEUE_OK;
}

// Function to read queue
QueueStatus Queue_Read(Queue *q, void *out, size_t out_cap, size_t *out_size)
{
    (void)q;
    (void)out;
    (void)out_cap;
    (void)out_size;
    return QUEUE_ERR_EMPTY;
}

// Function to close queue & prevent memory leaks
void Queue_Close(Queue *q)
{
    //1. Input validation
    if (q == NULL) {
        return;
    }
    
    //2. Free memory of buffer
    free(q->buffer);

    //3. Free memory of queue object structure
    free(q);
    
}
