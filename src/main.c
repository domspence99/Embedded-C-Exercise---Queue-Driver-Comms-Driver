#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "comm.h"



int main(void)
{
    //1. QUEUE INITIALISATION
    //Create queue pointer  
    Queue *q = NULL;
    
    //Initialise queue object q with capacity & set status
    QueueStatus init_status = Queue_Init(&q, 24);
    
    //CHECK INITIALISATION RETURNS STATUS
    if(init_status != QUEUE_OK){
        printf("Initialisation: FAILED (error %d)\n", init_status);
    }
    else{
        printf("Initialisation: PASSED\n");
        Queue_DebugPrintState(q); //print initialised state of q object
    }

    //2. QUEUE SEND
    Queue_Debug_TestSend(q, "HELLO", 5);
    Queue_Debug_TestSend(q, "ABC", 3);
    Queue_Debug_TestSend(q, "HI", 2);



    Queue_Close(q);
}

