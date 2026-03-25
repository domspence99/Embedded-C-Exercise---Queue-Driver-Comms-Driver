#ifndef COMM_H
#define COMM_H

#include <stddef.h>
#include <stdint.h>
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Communication status codes.
 */
typedef enum
{
    COMM_OK = 0,
    COMM_ERR_INVALID_ARG,
    COMM_ERR_NO_MEMORY,
    COMM_ERR_IO,
    COMM_ERR_EMPTY,
    COMM_ERR_BUFFER_TOO_SMALL
} CommStatus;

/**
 * @brief Message header used by the comm driver API.
 *
 * The driver will serialize/deserialize this into/from the underlying queue.
 */
typedef struct
{
    uint16_t command;      /**< Command or message ID */
    uint16_t length;       /**< Payload length in bytes */
    const uint8_t *data;   /**< Pointer to payload (ownership remains with caller) */
} CommMsg;

/**
 * @brief Optional RX callback type (bonus).
 *
 * Called when new data becomes available on RX side.
 */
typedef void (*CommRxCallback)(void *user_ctx);

/**
 * @brief Opaque comm handle.
 */
typedef struct Comm Comm;

/**
 * @brief Initialize a comm port emulator.
 * 
 * @details The comm port initialisation process is as follows:
 * - 2 Queues are initialised here, tx and rx
 * 
 * 1. Validate arguments
 * - Check that we receive a double pointer to initialise comm struct
 * - Check that tx and rx capacity are not 0
 * 2. Create space for a new comm instance
 * 3. Initialise 2 queues (tx and rx)
 * - Using the internal queue functions, initialise tx and rx & assign to 
 * new comm instance
 * 4. Check that the initialisation of the queues were successful
 * 5. Update pointer for comm with newly initialised comm
 * 6. Return success message
 * 
 * @param[out] comm          Receives allocated comm handle.
 * @param[in]  tx_capacity   TX queue capacity in bytes.
 * @param[in]  rx_capacity   RX queue capacity in bytes.
 *
 * @return COMM_OK on success.
 */
CommStatus Comm_Init(Comm **comm, size_t tx_capacity, size_t rx_capacity);

/**
 * @brief Register an RX callback (bonus).
 * 
 * @details
 * - Assigns the user-defined callback function & arguments to the comm 
 * instance.
 * - Called after bytes successfully sent into rx (emulated)
 *
 * If you implement this, call it when RX receives new bytes/message.
 */
CommStatus Comm_SetRxCallback(Comm *comm, CommRxCallback cb, void *user_ctx);

/**
 * @brief Send a message.
 * 
 * @details Implementation of Comm_Send is as follows:
 * 
 * The driver must serialize:
 *   [command(uint16_t)][length(uint16_t)][payload bytes]
 * into its TX path (queue).
 * 
 * This is done by using the internal QueueSend function. QueueSend requires
 * [payloadLength][payload] however the queue doesn't care what the payload is,
 * so in the payload we include the command, length (of desired data) and 
 * desired data. 
 * [payloadLength][command+length+data]
 * So this function creates new payload to send as [command+length+data] through
 * the internal queueSend function
 * 
 * 1. Validate inputs
 * 2. Calculate capacity (bytes) for new payload
 * 3. Create a buffer to store the new payload
 * 4. Copy the command message bytes into start of payload buffer
 * 5. Copy the length message bytes into payload buffer after command bytes
 * 6. Copy the data bytes after the command and length messages into the new payload
 * 7. Send the new payload to the tx queue
 * 8. Return success message
 */
CommStatus Comm_Send(Comm *comm, const CommMsg *msg);

/**
 * @brief Receive a message.
 *
 * Recommended pattern:
 * - Caller provides a payload buffer and capacity.
 * - Driver fills out_cmd/out_len and copies payload into buffer.
 * 
 * @details Comm Receive Implementation works as follows:
 * Queue read obtains the full message:
 * [length header][payload with command + length header + data payload]
 * Driver disects the second buffer and sets cmd, length and payload
 * 
 * 1. Validate inputs
 * 2. Create a buffer to read the message into using Queue Read
 * - [payload with command + length header + data payload]
 * 3. Perform QueueRead and copy contents of message into output buffer
 * 4. Disect the output buffer into command, length & data
 * - Copy the first 2 bytes to command pointer
 * - Copy the next two bytes to length pointer
 * - Copy the remaining data bytes for payload
 * 5. Return success 
 *
 * @param[in]     comm        Comm handle.
 * @param[out]    out_cmd     Receives command.
 * @param[out]    out_data    Destination buffer for payload.
 * @param[in]     out_cap     Capacity of out_data.
 * @param[out]    out_len     Receives payload length.
 *
 * @return
 *  - COMM_OK on success
 *  - COMM_ERR_EMPTY if no message available
 *  - COMM_ERR_BUFFER_TOO_SMALL if payload length > out_cap (message NOT removed)
 */
CommStatus Comm_Receive(Comm *comm,
                        uint16_t *out_cmd,
                        uint8_t *out_data,
                        size_t out_cap,
                        size_t *out_len);

/**
 * @brief Close the comm port and free resources.
 */
void Comm_Close(Comm *comm);

/**
 * @brief Emulate incoming RX data (test hook).
 * 
 * @details Implementation of Emulate Rx is as follows:
 *
 * This function simulates that bytes arrived from "hardware" into RX.
 * It should push the serialized message into the RX queue and trigger
 * the RX callback (if implemented).
 * 
 * EmulateRx simulates bytes arriving from hardware. 
 * I.e puts incoming bytes into the rx queue
 * Almost idential to the comm_send in this instance but pushes data to 
 * rx queue instead of tx queue
 * 
 * 1. Validate inputs
 * 2. Calculate capacity (bytes) for new payload
 * 3. Create a buffer to store the new payload
 * 4. Copy the command message bytes into start of payload buffer
 * 5. Copy the length message bytes into payload buffer after command bytes
 * 6. Copy the data bytes after the command and length messages into the new payload
 * 7. Send the new payload to the rx queue
 * 8. Return success message
 * 
 */
CommStatus Comm_EmulateRx(Comm *comm, const CommMsg *msg);


//---------ADDED HELPER FUNCTIONS---------

/**
 * @brief Helper function to print current state of comm port 
 * 
 * @param[in] q Queue instance
 */
void Comm_PrintCommState(Comm *comm);

/**
 * @brief Helper function to print contents of tx queue buffer 
 * 
 * @param[in] q Queue instance
 */
void Comm_PrintTXBuffer(Comm *comm);

/**
 * @brief Helper function to print contents of rx queue buffer 
 * 
 * @param[in] q Queue instance
 */
void Comm_PrintRXBuffer(Comm *comm);


#ifdef __cplusplus
}
#endif

#endif /* COMM_H */