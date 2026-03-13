#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Queue
{
    uint8_t *buffer; ///< Pointer to the buffer for data storage 
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
    if (q == NULL || (data == NULL && size != 0)){
        return QUEUE_ERR_INVALID_ARG;
    }

    //2. Calculate required bytes for message [2 byte header]+[payload bytes]
    size_t bytesRequired = sizeof(uint16_t) + size;


    //3. Calculate total remaining available bytes (can be at front or end)
    size_t availableBytes = q->capacity - q->bytesUsed; 
    printf("Bytes available: %zu, Bytes required: %zu\n",availableBytes,bytesRequired);
    
    //4. Check if queue has space (prevents overflow)
    if (bytesRequired > availableBytes){
        printf("Queue FULL\n");
        return QUEUE_ERR_FULL;
    }
    
    
   //-----------------WRITING HEADER -----------------
    //1. Set 2 byte header message 
    uint16_t header_msg = size;  
    printf("Header message: %04X\n", header_msg);
    
    //2. Calculate Number of bytes between head & end of buffer
    size_t availableEndSpace = q->capacity - q->head;
    printf("Bytes available between head & end of buffer: %zu\nBytes of header: %zu\n", availableEndSpace,sizeof(header_msg));
    
    
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
    
    return QUEUE_OK;
}

// Function to read queue
QueueStatus Queue_Read(Queue *q, void *out, size_t out_cap, size_t *out_size)
{
    //out - buffer where payload will be copied
    //out_cap - size of output buffer
    //out_size - how many bytes were read
    //1. Validate arguments
    if (q == NULL || out == NULL || out_size == NULL){
        return QUEUE_ERR_INVALID_ARG;
    }

    //2. Check if queue is empty (nothing inside the queue)
    if (q->bytesUsed == 0){
        return QUEUE_ERR_EMPTY;
    }

    //3. READ HEADER LENGTH
    //Header size
    uint16_t payloadLength;

    //Determine how many bytes are available before wrap
    size_t availableEndSpace = q->capacity - q->tail;

    //Limit to header size
    if (availableEndSpace > sizeof(uint16_t)){  
        availableEndSpace = sizeof(uint16_t);
    }

    //Copy header
    memcpy(&payloadLength, q->buffer + q->tail, availableEndSpace);

    //Copy payload
    memcpy(((uint8_t *)&payloadLength) + availableEndSpace, q->buffer, sizeof(uint16_t) - availableEndSpace);

    //4. MOVE TAIL PAST HEADER
    q->tail = (q->tail + sizeof(uint16_t)) % q->capacity;

    //5. CHECK OUTPUT BUFFER SIZE
    //If buffer can't hold payload, return error
    if (payloadLength > out_cap){
        return QUEUE_ERR_BUFFER_TOO_SMALL;
    }

    //6. Read payload
    //Convert output to a byte pointer
    uint8_t *bytes = (uint8_t *)out;

    //7. How many payloads fit before wrap
    availableEndSpace = q->capacity - q->tail;
    if (availableEndSpace > payloadLength){
        availableEndSpace = payloadLength;
    }

    //8. Copy availableEndSpace part 
    memcpy(bytes, q->buffer + q->tail, availableEndSpace);

    //9. Copy second part (if needed)
    memcpy(bytes + availableEndSpace, q->buffer, payloadLength - availableEndSpace);

    //10. Advance tail past payload
    q->tail = (q->tail + payloadLength) % q->capacity;

    //11. Reduce amount of bytes inside buffer (removed)
    q->bytesUsed -= sizeof(uint16_t) + payloadLength;

    //12. Return payload size (how many bytes were read)
    *out_size = payloadLength;

    //DEBUG OUTPUT
    /*
    printf("Buffer after read: ");
    for (size_t i = 0; i < q->capacity; i++)
    {
        printf("%02X ", q->buffer[i]);
    }
    printf("\n");

    printf("head=%zu tail=%zu bytesUsed=%zu\n", q->head, q->tail, q->bytesUsed);
    */
    //13. return success
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

void Queue_Debug_PrintInitialBuffer(const Queue *q){
    //1. Check queue object is there
    if (q == NULL)
    {
        printf("Queue is NULL\n");
        return;
    }

    //2. Print queue struct
    printf("Queue state after initialization:\n");
    printf("Capacity : %zu\n", q->capacity);
    printf("Head     : %zu\n", q->head);
    printf("Tail     : %zu\n", q->tail);
    printf("bytesUsed    : %zu\n", q->bytesUsed);

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

//Helper function to print Buffer
void Queue_DebugPrintBuffer(const Queue *q)
{
    //Takes in queue object
    if (q == NULL)
        return;

    //Prints out the data in buffer
    printf("Buffer: ");

    for (size_t i = 0; i < q->capacity; i++)
    {
        printf("%02X ", q->buffer[i]);
    }

    printf("\n");
    printf("head=%zu tail=%zu bytesUsed=%zu\n", q->head, q->tail, q->bytesUsed);
}


//Helper function to send bytes
void Queue_Debug_TestSend(Queue *q, const void *data, size_t len)
{
    
    printf("TEST SEND\nSending %zu bytes: ", len);

    QueueStatus status = Queue_Send(q, data, len);

    //Check that status of the send is valid
    if (status != QUEUE_OK)
        printf("FAIL (status=%d)\n", status);
    else
        printf("PASS\n");
        Queue_DebugPrintBuffer(q); //print buffer
}

//Helper function to read bytes (INCOMPLETE)
void Queue_Debug_TestRead(Queue *q){
    //DEBUG OUTPUT
    printf("TEST READ:\n");

    //TODO Check if read returns errors

    printf("Buffer: ");

    for (size_t i = 0; i < q->capacity; i++)
    {
        printf("%02X ", q->buffer[i]);
    }
    printf("\n");

    printf("head=%zu tail=%zu bytesUsed=%zu\n\n", q->head, q->tail, q->bytesUsed);
}