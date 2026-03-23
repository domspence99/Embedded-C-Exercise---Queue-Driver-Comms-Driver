#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Queue
{
    uint8_t *buffer; ///< Pointer to the buffer for data storage (in bytes)
    size_t capacity; ///< Total size of buffer in bytes
    size_t head;     ///< Index of head position (write pointer)
    size_t tail;     ///< Index of tail position (read pointer)
    size_t bytesUsed;    ///< Counter for number of occupied bytes in buffer
};


QueueStatus Queue_Init(Queue **q, size_t capacity)
{
    //1. Validate inputs
    if (q == NULL || capacity == 0)
    {
        return QUEUE_ERR_INVALID_ARG;
    }
    //2. Create a new queue instance
    Queue *new_queue = malloc(sizeof(Queue)); 

    //3. Check if memory was allocated
    if (new_queue == NULL)
    {
        return QUEUE_ERR_NO_MEMORY;
    }

    //4. Allocate memory for buffer of size:capacity (bytes)
    new_queue->buffer = malloc(capacity);
    
    //5. Check if memory was allocated for buffer
    if (new_queue->buffer == NULL)
    {
        free(new_queue); 
        return QUEUE_ERR_NO_MEMORY;
    }

    //6. Initialise the queue fields
    new_queue->capacity = capacity; 
    new_queue->head = 0;
    new_queue->tail = 0;
    new_queue->bytesUsed = 0;

    //7. Assign new created queue in memory
    *q = new_queue;

    //8. Return success status
    return QUEUE_OK;
}

// Function to send message to queue
QueueStatus Queue_Send(Queue *q, const void *data, size_t size)
{
    //-----------------VALIDATING INPUT-----------------
    //1. Validate arguments
    if (q == NULL || data == NULL){
        printf("ERROR: Invalid Arguments\n");
        return QUEUE_ERR_INVALID_ARG;
    }

    //2. Calculate required bytes for message [2 byte header]+[payload bytes]
    size_t bytesRequired = sizeof(uint16_t) + size;


    //3. Calculate total remaining available bytes (can be at front or end)
    size_t availableBytes = q->capacity - q->bytesUsed; 
    
    //4. Check if queue has space (prevents overflow)
    if (bytesRequired > availableBytes){
        printf("Queue Send: FAIL (Required bytes: %lu, Available bytes: %lu)\n", bytesRequired, availableBytes);
        return QUEUE_ERR_FULL;
    }
    
    
   //-----------------WRITING HEADER -----------------
    //1. Set 2 byte header message 
    uint16_t header_msg = size;  
    
    //2. Calculate Number of bytes between head & end of buffer
    size_t availableEndSpace = q->capacity - q->head;
    
    
    //3. Limit availableEndSpace variable between (0-2)
    if (availableEndSpace > sizeof(header_msg)){
        availableEndSpace = sizeof(header_msg); 
    }

    //4. Copy at the head, availableEndSpace amount of bytes of the header message (0-2) 
    memcpy(q->buffer + q->head, &header_msg, availableEndSpace);
    
    //5. Copy at the start of buffer, the remaining header message bytes (0-2)
    memcpy(q->buffer, ((uint8_t*)&header_msg) + availableEndSpace, sizeof(header_msg) - availableEndSpace);

    //6. Advance the head pointer (% ensures pointer wraps around)
    q->head = (q->head + sizeof(header_msg)) % q->capacity;

   
    //-----------------WRITING PAYLOAD -----------------
    //1. Convert void data pointer into byte pointer
    const uint8_t *payloadBytes = (const uint8_t *)data;   
    
    //2. Recalculate available endSpace
    availableEndSpace = q->capacity - q->head;
    
    //3. Limit number of bytes to send to size of payload 
    if(availableEndSpace > size){
        availableEndSpace = size;
    } 
    
    //4. Copy at the head, availableEndSpace amount of bytes of the payload message (0-size)
    memcpy(q->buffer + q->head, payloadBytes, availableEndSpace);  
    
    //5. Copy at the start of buffer, the remaining payload bytes (0-size)
    memcpy(q->buffer, payloadBytes + availableEndSpace, size-availableEndSpace); 

    //6. Advance the head index again 
    q->head = (q->head + size) % q->capacity;

    //7. Update occupied bytes in queue (header+payload)
    q->bytesUsed += bytesRequired;
    
    printf("Queue Send: SUCCESS (Sent %zu bytes)\n", bytesRequired);
    return QUEUE_OK;
}

// Function to read queue
QueueStatus Queue_Read(Queue *q, void *out, size_t out_cap, size_t *out_size)
{
    //Arguments:
    //q (INPUT) - queue instance to read from
    //*out (OUTPUT) - output buffer to store read bytes
    //out_cap (INPUT) - size of output buffer (bytes)
    //*out_size (OUTPUT) - number of bytes read 
    
    //-----------------VALIDATING INPUT-----------------
    //1. Validate arguments
    if (q == NULL || out == NULL || out_size == NULL){
        return QUEUE_ERR_INVALID_ARG;
    }

    //2. Check if queue is empty (nothing inside the queue)
    if (q->bytesUsed == 0){
        return QUEUE_ERR_EMPTY;
    }

   //-----------------READING HEADER LENGTH -----------------
    //1. Create 2 byte variable for header
    uint16_t header_msg;

    //2. Determine how many bytes are available between end of buffer and tail
    size_t availableEndSpace = q->capacity - q->tail;

    //3. Limit number of bytes to read of the header
    if (availableEndSpace > sizeof(uint16_t)){  
        availableEndSpace = sizeof(uint16_t);
    }

    //4. Copy n(availableEndSpace) amount of bytes to the header_msg variable at the tail
    memcpy(&header_msg, q->buffer + q->tail, availableEndSpace);

    //5. Copy the rest of the header message bytes (if wrapped at end)
    //If there was not enough space at the end of the buffer, the header was wrapped
    //Therefore, copy the extra header byte from the start of the buffer
    memcpy(((uint8_t *)&header_msg) + availableEndSpace, q->buffer, sizeof(uint16_t) - availableEndSpace);

    //6. Advance the tail past the header (wrap if needed)
    q->tail = (q->tail + sizeof(uint16_t)) % q->capacity;

    //-----------------READING PAYLOAD -----------------
    //1. Check if output buffer can hold the amount of payload bytes required
    if (header_msg > out_cap){
        return QUEUE_ERR_BUFFER_TOO_SMALL;
    }

    //2. Convert void output buffer type to a byte pointer
    uint8_t *payloadOutputBytes = (uint8_t *)out;

    //3. Limit bytes needed to size of payload (read earlier)
    availableEndSpace = q->capacity - q->tail;
    if (availableEndSpace > header_msg){
        availableEndSpace = header_msg;
    }

    //4. Copy n(availableEndSpace) amount of bytes into payload output buffer at the tail 
    memcpy(payloadOutputBytes, q->buffer + q->tail, availableEndSpace);

    //5. Copy reamining bytes of payload (at start of queue buffer) into output buffer (if wrapped)
    memcpy(payloadOutputBytes + availableEndSpace, q->buffer, header_msg - availableEndSpace);

    //6. Advance tail past payload
    q->tail = (q->tail + header_msg) % q->capacity;

    //7. Reduce amount of bytes used inside buffer 
    q->bytesUsed -= sizeof(uint16_t) + header_msg;

    //8. Updates payload size (how many bytes were read)
    *out_size = header_msg;

    //9. Return success
    printf("Queue Read: SUCCESS (Read %hu bytes)\n",header_msg);
    return QUEUE_OK;
}

// Function to close queue & prevent memory leaks
void Queue_Close(Queue *q)
{
    //1. Check a valid queue object is provided
    if (q == NULL) {
        return;
    }
    
    //2. Free memory of buffer
    free(q->buffer);

    //3. Free memory of queue object structure
    free(q);
    
}

void Queue_PrintQueueState(const Queue *q){
    //1. Check queue object is valid
    if (q == NULL)
    {
        printf("Queue is NULL\n");
        return;
    }

    //2. Print queue struct state
    printf("Current queue state:\n");
    printf("Capacity    : %zu\n", q->capacity);
    printf("Head        : %zu\n", q->head);
    printf("Tail        : %zu\n", q->tail);
    printf("bytesUsed   : %zu\n\n", q->bytesUsed);
}

void Queue_PrintQueueBuffer(const Queue *q){
    //1. Check queue object is there
    if (q == NULL)
    {
        printf("Print Buffer: ERROR (Queue is NULL)\n\n");
        return;
    }
    printf("Buffer contents:\n");

    //3. Print each byte of buffer
    for (size_t i = 0; i < q->capacity; i++)
    {
        printf("%02X ", q->buffer[i]);

        if ((i + 1) % 16 == 0) //after 16 bytes create new line
            printf("\n");
    }

    printf("\n\n");
}

void Queue_PrintOutputBuffer(uint8_t *buffer, size_t buffer_size){
    printf("READ BUFFER:\n");
    for(size_t i=0;i<buffer_size;i++){
        printf("%02X ",buffer[i]); 
        if ((i + 1) % 16 == 0){ //after 16 bytes create new line
        printf("\n");
        }
    }
    printf("\n\n");
}


