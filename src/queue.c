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
    //1. Validate arguments
    if (q == NULL || (data == NULL && size != 0)){
        return QUEUE_ERR_INVALID_ARG;
    }

    //2. Calculate required bytes for message [2 byte header]+[payload bytes]
    size_t bytesRequired = sizeof(uint16_t) + size;
    //3. Calculate remaining available bytes
    size_t availableBytes = q->capacity - q->bytesUsed; 
    printf("Bytes available: %zu, Bytes required: %zu\n",availableBytes,bytesRequired);
    //Set 2 byte header message 
    uint16_t header_msg = size; 
    printf("Header message: %04X\n", header_msg);
    //Convert void data pointer into byte pointer
    const uint8_t *payloadBytes = (const uint8_t *)data;

    //3. Check if queue has enough space for next message 
    if(availableBytes >= bytesRequired){
        //WRITE AT HEAD
        printf("Writing at head\n");
        //Copy at head position of buffer, the header message (can be up to 2 bytes) 
        memcpy(q->buffer + q->head, &header_msg,sizeof(uint16_t));
        //Advance the head to after the header message
        q->head += sizeof(uint16_t);
        //Copy all of the payload bytes into the buffer at the head
        memcpy(q->buffer + q->head, payloadBytes, size);
        //Advance the head to after the payload
        q->head += size;
        //Update bytes used
        q->bytesUsed += bytesRequired;

        //NOT YET CORRECT, BECAUSE
        //Bytes can be available at the start if they have already been read & discarded
        //Therefore, available bytes could be 2 at end, 2 at start (available >= required is true)
        //therefore, need to copy first part end, wrap head then second part to start

    }
    else{
        //Bytes can be available at the start if they have already been read
        //Queue is full if not enough space at front or back
        printf("Queue FULL\n");
        return QUEUE_ERR_FULL;
    }



    /*/

    //4. WRITING HEADER (payload length)
    uint16_t payloadLength = size;
     //Restricts payloadLength to max payload size (65535)
     
    
    
    //Write up to size of header
    if (first > sizeof(uint16_t)){
        first = sizeof(uint16_t);
    }

    //copy first part of the header into buffer
    memcpy(q->buffer + q->head, &payloadLength, first);
    
    //if header is wrapped around, copy remaining bytes to beginning of buffer
    memcpy(q->buffer, ((uint8_t*)&payloadLength) + first, sizeof(uint16_t) - first);

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
    q->bytesUsed += required;


    //DEBUG OUTPUT
    
    printf("Buffer: ");
    for (size_t i = 0; i < q->capacity; i++){
        printf("%02X ", q->buffer[i]);
    }
    printf("\n");
    printf("head=%zu tail=%zu bytesUsed=%zu\n", q->head, q->tail, q->bytesUsed);
    */
    
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
    size_t first = q->capacity - q->tail;

    //Limit to header size
    if (first > sizeof(uint16_t)){  
        first = sizeof(uint16_t);
    }

    //Copy header
    memcpy(&payloadLength, q->buffer + q->tail, first);

    //Copy payload
    memcpy(((uint8_t *)&payloadLength) + first, q->buffer, sizeof(uint16_t) - first);

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
    first = q->capacity - q->tail;
    if (first > payloadLength){
        first = payloadLength;
    }

    //8. Copy first part 
    memcpy(bytes, q->buffer + q->tail, first);

    //9. Copy second part (if needed)
    memcpy(bytes + first, q->buffer, payloadLength - first);

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
        printf("%04X ", q->buffer[i]);

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
        printf("%04X ", q->buffer[i]);
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
        printf("%04X ", q->buffer[i]);
    }
    printf("\n");

    printf("head=%zu tail=%zu bytesUsed=%zu\n\n", q->head, q->tail, q->bytesUsed);
}