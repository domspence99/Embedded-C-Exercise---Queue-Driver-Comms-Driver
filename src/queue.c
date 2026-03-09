#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Function to send message to queue
QueueStatus Queue_Send(Queue *q, const void *data, size_t size)
{
    //1. Validate arguments
    //Ensure queue pointer and data pointer are valid
    if (q == NULL || data == NULL){
        return QUEUE_ERR_INVALID_ARG;
    }

    //2. Calculate required space 
    // required space is [2 byte header]+[payload bytes] 
    size_t required = sizeof(uint16_t) + size; //


    //3. Check if queue has space (prevents overflow)
    //Free space = capicity - space already used
    if (required > (q->capacity - q->count)){
        printf("Queue FULL\n");
        return QUEUE_ERR_FULL;
    }

    //4. WRITING HEADER (payload length)
    uint16_t msg_len = size;

    //Determine how many bytes available before reaching end of buffer
    size_t first = q->capacity - q->head;
    
    //Write up to size of header
    if (first > sizeof(uint16_t)){
        first = sizeof(uint16_t);
    }

    //copy first part of the header into buffer
    memcpy(q->buffer + q->head, &msg_len, first);
    
    //if header is wrapped around, copy remaining bytes to beginning of buffer
    memcpy(q->buffer, ((uint8_t*)&msg_len) + first, sizeof(uint16_t) - first);

    //Advance the head pointer (% ensures pointer wraps around)
    q->head = (q->head + sizeof(uint16_t)) % q->capacity;


    //5. WRITING PAYLOAD
    //Convert void data pointer into byte pointer
    const uint8_t *bytes = (const uint8_t *)data;

    //Calculate how many payload cycles before reaching end of buffer
    first = q->capacity - q->head;
    
    //If payload fits, limit to copy size
    if (first > size){
        first = size;
    }

    //Copy first part of payload into buffer
    memcpy(q->buffer + q->head, bytes, first);

    //If payload wraps, copy remaining bits into start of buffer
    memcpy(q->buffer, bytes + first, size - first);

    //Advance header pointer
    q->head = (q->head + size) % q->capacity;

    //Update bytes currently stored in queue
    q->count += required;


    //DEBUG OUTPUT
    printf("Buffer: ");
    for (size_t i = 0; i < q->capacity; i++){
        printf("%02X ", q->buffer[i]);
    }
    printf("\n");
    printf("head=%zu tail=%zu count=%zu\n", q->head, q->tail, q->count);
    
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
