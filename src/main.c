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
    
    //Initialise queue object q with n bytes of capacity & set status
    QueueStatus init_status = Queue_Init(&q, 10);
    
    //CHECK INITIALISATION RETURNS STATUS
    if(init_status != QUEUE_OK){
        printf("Initialisation: FAILED (error %d)\n", init_status);
    }
    else{
        printf("Initialisation: PASSED\n");
        Queue_Debug_PrintInitialBuffer(q); 
    }

    //2. SEND DATA TO BUFFER
    const uint8_t payload[] = {0x10, 0x20, 0x30};
    uint16_t payloadSize = (uint16_t)sizeof(payload);

    printf("%d",Queue_Send(q,payload,payloadSize));
    /*

    //2. QUEUE SEND & READ
    Queue_Debug_TestSend(q, "HELLO", 5);
    Queue_Debug_TestRead(q);
    Queue_Debug_TestSend(q, "ABC", 3);
    Queue_Debug_TestRead(q);
    Queue_Debug_TestSend(q, "HI", 2);
    Queue_Debug_TestRead(q);


    //3. CLOSE QUEUE OBJECT TO PREVENT MEMORY LEAKS
    */
    Queue_Close(q);
}

