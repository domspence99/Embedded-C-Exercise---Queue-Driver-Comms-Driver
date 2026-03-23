#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Comm struct contains 2 queue intances, for tx and rx
struct Comm {
    Queue *tx;
    Queue *rx;
};

CommStatus Comm_Init(Comm **comm, size_t tx_capacity, size_t rx_capacity)
{    
    //1. Validate arguments
    if (comm == NULL || tx_capacity == 0 || rx_capacity == 0){
        printf("COMM_ERR_INVALID_ARG\n");
        return COMM_ERR_INVALID_ARG;
    }

    //2. Create space for a new comm instance
    Comm *new_comm = malloc(sizeof(Comm));


    //3. Initialise 2 queues, tx and rx to the new comm instance
    QueueStatus txInitStatus = Queue_Init(&new_comm->tx,tx_capacity);
    QueueStatus rxInitStatus = Queue_Init(&new_comm->rx,rx_capacity);

    //4. Check that the initialisation of the queues were successful
    if(txInitStatus != QUEUE_OK){
        printf("COMM TX QUEUE INITIALISATION ERROR: %d\n", txInitStatus);
        return COMM_ERR_INVALID_ARG;
    }
    else if(rxInitStatus != QUEUE_OK){
        printf("COMM RX QUEUE INITIALISATION ERROR: %d\n", rxInitStatus);
        return COMM_ERR_INVALID_ARG;
    }

    //5. Update pointer for comm with newly initialised comm
    *comm = new_comm;
    return COMM_OK;
}

CommStatus Comm_SetRxCallback(Comm *comm, CommRxCallback cb, void *user_ctx)
{
    (void)comm;
    (void)cb;
    (void)user_ctx;
    return COMM_OK;
}

CommStatus Comm_Send(Comm *comm, const CommMsg *msg)
{

    //1. Validate arguments
    if(comm == NULL || msg == NULL){
        printf("ERROR: Comm Send Invalid Arguments\n");
        return COMM_ERR_INVALID_ARG;
    }

    //2. Calculate capacity (bytes) for new payload
    size_t payload_capacity = sizeof(msg->command) + sizeof(msg->length) + msg->length;

    //3. Create a buffer to store the new payload
    uint8_t new_payload[payload_capacity];

    //4. Copy the command message bytes into start payload buffer
    memcpy(new_payload, &msg->command, sizeof(msg->command));

    //5. Copy the length message bytes into payload buffer after command bytes
    memcpy(new_payload + sizeof(msg->command), &msg->length, sizeof(msg->length));

    //6. Copy the data bytes after the command and length messages into the new payload
    memcpy(new_payload + sizeof(msg->length) + sizeof(msg->command), msg->data, msg->length);

    printf("Sending new payload:\n");
    for(size_t i=0;i<payload_capacity;i++){
        printf("%02X",new_payload[i]);
    }
    printf("\n\n");

    //7. Send the new payload to the tx queue
    Queue_Send(comm->tx,new_payload,payload_capacity);

    return COMM_OK;
}

CommStatus Comm_Receive(Comm *comm,
                        uint16_t *out_cmd,
                        uint8_t *out_data,
                        size_t out_cap,
                        size_t *out_len)
{
    
    //1. Validate inputs
    if(comm==NULL || out_cmd==NULL || out_data==NULL || out_len==NULL){
        printf("ERROR Comm Receive: Invalid Arguments");
        return COMM_ERR_INVALID_ARG;
    }

    //2. Create a buffer to read the message into
    uint8_t output_buffer[out_cap];
    memset(output_buffer, 0, out_cap); //Initialize data output buffer to 0
    
    //3. Perform QueueRead and copy contents of message into output buffer
    if(Queue_Read(comm->rx,output_buffer,out_cap,out_len)!=QUEUE_OK){
        printf("COMM RECEIVE: ERROR\n");
        return COMM_ERR_IO;
    };

    //4. Disect the output buffer into command, length & data
    //4.1 Copy the first 2 bytes to command pointer
    memcpy(out_cmd, output_buffer, sizeof(uint16_t));
    //4.2 Copy the next two bytes to length pointer
    memcpy(out_len, output_buffer+sizeof(uint16_t), sizeof(uint16_t));
    //4.3 Copy the remaining data bytes for payload
    memcpy(out_data, output_buffer+(sizeof(uint16_t)*2), out_cap-(sizeof(uint16_t)*2));

    //5. Return success 
    return COMM_OK;
}

CommStatus Comm_EmulateRx(Comm *comm, const CommMsg *msg)
{
    
    //1. Validate arguments
    if(comm == NULL || msg == NULL){
        printf("ERROR: Comm EmulateRx Invalid Arguments\n");
        return COMM_ERR_INVALID_ARG;
    }

    //2. Calculate capacity (bytes) for new payload
    size_t payload_capacity = sizeof(msg->command) + sizeof(msg->length) + msg->length;
    printf("Payload capacity: %zu\n",payload_capacity);

    //3. Create a buffer to store the new payload
    uint8_t new_payload[payload_capacity];

    //4. Copy the command message bytes into start payload buffer
    memcpy(new_payload, &msg->command, sizeof(msg->command));

    //5. Copy the length message bytes into payload buffer after command bytes
    memcpy(new_payload + sizeof(msg->command), &msg->length, sizeof(msg->length));

    //6. Copy the data bytes after the command and length messages into the new payload
    memcpy(new_payload + sizeof(msg->length) + sizeof(msg->command), msg->data, msg->length);
    
    printf("Emulating new rx payload:\n");
    for(size_t i=0;i<payload_capacity;i++){
        printf("%02X",new_payload[i]);
    }
    printf("\n\n");

    //7. Send the new payload to the rx queue
    Queue_Send(comm->rx,new_payload,payload_capacity);
    return COMM_OK;
}

void Comm_Close(Comm *comm)
{
    //Close memory allocations to prevent memory leaks
    if (comm== NULL) {
        return;
    }

    //Close queues
    Queue_Close(comm->tx);
    Queue_Close(comm->rx);
    
    //Close comm
    free(comm);
}

void Comm_PrintCommState(Comm *comm){
     //1. Check queue object is valid
    if (comm == NULL)
    {
        printf("ERROR: Can't print comm state, comm is NULL\n");
        return;
    }

    //2. Print comm struct state
    printf("Current tx state:\n");
    Queue_PrintQueueState(comm->tx);
    printf("Current tx state:\n");
    Queue_PrintQueueState(comm->rx);

}
void Comm_PrintTXBuffer(Comm *comm){
    if (comm == NULL){
        printf("ERROR: Can't print TX, comm is NULL\n");
        return;
    }
    printf("TX BUFFER\n");
    Queue_PrintQueueBuffer(comm->tx);
}

void Comm_PrintRXBuffer(Comm *comm){
    if (comm == NULL){
        printf("ERROR: Can't print RX, comm is NULL\n");
        return;
    }
    printf("RX BUFFER\n");
    Queue_PrintQueueBuffer(comm->rx);
}


