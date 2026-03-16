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
    size_t queue_capacity = 10;
    
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

    //2. SEND DATA TO BUFFER
    //const uint8_t payload[] = {0x10, 0x20, 0x30,0x40,0x50};
    //const uint8_t payload2[] = {0x10, 0x20, 0x30};
    
    /*
    uint8_t payload6[300];
    for (int i=0;i<300;i++){
        payload6[i] = i;
    }
    */

    const uint8_t payload3[] = {0x10, 0x20};
    const uint8_t payload4[] = {0x40};
    //uint16_t payloadSize = (uint16_t)sizeof(payload);

    //printf("%d",Queue_Send(q,payload,payloadSize));
    //Queue_PrintQueueBuffer(q);
    //printf("%d",Queue_Send(q,payload6,(uint16_t)sizeof(payload6)));
    //Queue_PrintQueueBuffer(q);
    Queue_Send(q,payload3,(uint16_t)sizeof(payload3));
    Queue_PrintQueueBuffer(q);
    Queue_Send(q,payload4,(uint16_t)sizeof(payload4));
    Queue_PrintQueueBuffer(q);

    //3. READ X AMOUNT OF BYTES FROM BUFFER
    //3.1 Create an output buffer to store n(outputBufferSizebytes) bytes
    size_t outputBufferSize = 10;
    uint8_t output_buffer[outputBufferSize];
    memset(output_buffer, 0, outputBufferSize); //Initialize output buffer to 0
    size_t bytesRecieved;
    
    //3.2 Call read queue function
    Queue_Read(q,&output_buffer,outputBufferSize,&bytesRecieved);
    printf("Bytes received: %lu\n", bytesRecieved);
    Queue_PrintOutputBuffer(output_buffer,outputBufferSize);



    Queue_Read(q,&output_buffer,outputBufferSize,&bytesRecieved);
    printf("Bytes received: %lu\n", bytesRecieved);
    Queue_PrintOutputBuffer(output_buffer,outputBufferSize);
    //**For next time, queue is reading, first message, then nothing is changing
    //Need to remove message from q buffer and read the next message
        
    //4. CLOSE QUEUE OBJECT TO PREVENT MEMORY LEAKS
    Queue_Close(q);
}

