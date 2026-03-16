/*
 * Description:
 * Implementation of a circular byte queue used to store variable length
 * messages. Each message is stored with a length header followed by
 * the payload.
 *
 * Work Completed:
 * - Implemented Queue_Init() to allocate and initialise queue structure.
 * - Implemented Queue_Send() to store variable-length messages.
 * - Implemented Queue_Read() to retrieve messages in FIFO order.
 * - Implemented Queue_Close() to free allocated memory.
 * - Added helper test functions in main() to validate queue behaviour.
 *
 * Testing Performed:
 * - Verified queue initialization.
 * - Tested sending multiple messages.
 * - Tested reading messages in FIFO order.
 * - Confirmed message data integrity after send/read.
 *
 * Future Work / Improvements:
 * Queue Driver:
 * - Finish (QueueRead tests)
 * - Add more case tests (queue full, queue empty).
 * - Test circular buffer wrap-around behaviour.
 * - Add error handling tests for invalid parameters.
 * - Separate test code from implementation.
 * 
 * Comms Driver:
 * - Combine the completed queue driver along with the
 * implementation of a comms driver
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "comm.h"

int main(void)
{
    //1. QUEUE INITIALISATION
    //Create queue pointer  
    Queue *q = NULL;
    size_t queue_capacity = 16;
    
    //Initialise queue object q with n bytes of capacity & set status
    QueueStatus init_status = Queue_Init(&q, queue_capacity);
    
    //CHECK INITIALISATION RETURNS STATUS
    if(init_status != QUEUE_OK){
        printf("Initialisation: FAILED (error %d)\n", init_status);
    }
    else{
        printf("Initialisation: PASSED\n");
        Queue_PrintQueueState(q);
        Queue_PrintQueueBuffer(q); 
    }

    //2. SEND AND READ FROM BUFFER
    //Variable length messages
    const uint8_t payload[] = {0x10};
    const uint8_t payload2[] = {0x11, 0x12};
    const uint8_t payload3[] = {0x13, 0x14, 0x15};
    const uint8_t payload4[] = {0x16, 0x17, 0x18, 0x19};

    //Output buffer initialisation
    size_t outputBufferSize = queue_capacity; //Output buffer same capacity as queue buffer
    uint8_t output_buffer[outputBufferSize];  //Create output buffer to store bytes
    memset(output_buffer, 0, outputBufferSize); //Initialize output buffer to 0
    size_t bytesRecieved;

    //SEND
    Queue_Send(q,payload,sizeof(payload));  //Send 3 bytes
    Queue_PrintQueueBuffer(q);
    Queue_Send(q,payload2,(uint16_t)sizeof(payload2)); //Send 4 bytes
    Queue_PrintQueueBuffer(q);
    Queue_Send(q,payload3,(uint16_t)sizeof(payload3)); //Send 5 bytes
    Queue_PrintQueueBuffer(q);
    Queue_Send(q,payload4,(uint16_t)sizeof(payload4)); //FAILS (Requires 6 bytes, only 4 available)
    Queue_PrintQueueBuffer(q);

    //READ
    Queue_Read(q,&output_buffer,outputBufferSize,&bytesRecieved); //Reads first message (adds 1 byte to output & removes 3 bytes from queue)
    printf("Bytes received: %lu\n", bytesRecieved);
    Queue_PrintOutputBuffer(output_buffer,outputBufferSize);

    Queue_Read(q,&output_buffer,outputBufferSize,&bytesRecieved); //Reads second message (overwrites 2 bytes to output buffer & removes 4 bytes from queue)
    printf("Bytes received: %lu\n", bytesRecieved);
    Queue_PrintOutputBuffer(output_buffer,outputBufferSize);
    
    //OVERWRITE (WRAP AROUND)
    Queue_Send(q,payload4,(uint16_t)sizeof(payload4)); //Sends 6 bytes, wraps back around
    Queue_PrintQueueBuffer(q);
    
    //4. CLOSE QUEUE OBJECT TO PREVENT MEMORY LEAKS
    Queue_Close(q);
}

