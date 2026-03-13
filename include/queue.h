#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Queue error codes.
 */
typedef enum
{
    QUEUE_OK = 0,
    QUEUE_ERR_INVALID_ARG,
    QUEUE_ERR_NO_MEMORY,
    QUEUE_ERR_FULL,
    QUEUE_ERR_EMPTY,
    QUEUE_ERR_BUFFER_TOO_SMALL
} QueueStatus;

/**
 * @brief Opaque queue handle.
 *
 * Implementation can be a ring-buffer in bytes, storing:
 * [uint16_t length][payload bytes] repeated.
 */
typedef struct Queue Queue;

/**
 * @brief Create/initialize a queue.
 *
 * @details The initialisation process is as follows:
 * 1. Validate arguments
 * - check that the address of the q object is not a NULL pointer & that a 
 * capacitity parameter is supplied.
 * 
 * 2. Create a new queue instance
 * - Create a new instance and allocate memory for queue structure.
 * 
 * 3. Check if memory was successfully allocated 
 * 
 * 4. Allocate memory for buffer with size(capacity)
 * 
 * 5. Check if memory was allocated for buffer 
 * - If not enough memory for buffer, clear the entire instance to prevent a 
 * memory leak.
 * 
 * 6. Initialise the queue fields
 * 
 * 7. Asign the newly created queue instance to to the pointer in memory
 * 
 * 8. Return success status

 * 
 * @param[out] q         Pointer that will receive allocated queue handle.
 * @param[in]  capacity  Capacity in bytes for internal storage.
 *
 * @return 
 * - QUEUE_OK on success
 * - QUEUE_ERR_INVALID_ARG if NULL pointers or no capacity supplied 
 * - QUEUE_ERR_NO_MEMORY if no memory available for strucure object or buffer
 *  
 */
QueueStatus Queue_Init(Queue **q, size_t capacity);

/**
 * @brief Enqueue a message.
 *
 * The queue must copy the payload bytes into its internal buffer.
 *
 * @details The queue send message process is as follows:
 * 1. Validate arguments
 * - Check for valid queue object and that data is not NULL unless size is 0
 * 
 * 2. Calculate required bytes for message
 * - Required bytes = Header + Payload 
 * - [2 bytes to represent length of payload][Number of bytes of payload ]
 * 
 * 3. Calculate total remaining available bytes
 * - Used to check if there is enough space in queue
 * - Available bytes left = capicity of buffer - bytes already used
 * 
 * 4. Check if queue has enough space for next message 
 * - prevents overflow & overwriting of unread data
 * - returns QUEUE_ERR_FULL if not enough space
 * 
 * -----------------WRITING HEADER-----------------
 * 1. Set 2 byte header message 
 * - 2 byte header restricts payloadLength max size to (65535)
 * 
 * 2. Calculate number of bytes between head & end of buffer
 * 
 * 3. Limit availableEndSpace variable between (0-2)
 * - If the bytes available between head and end of buffer are greater than the 
 * size of the header we have enough space to put the header so we limit the
 * variable to 2 bytes. If not the availableEndSpace variable will be less than 2
 *  
 * 4. Copy at the head, availableEndSpace amount of bytes of the header message (0-2)
 * - Slice 1: If we have enough space, availableEndSpace set to 2 and bytes are 
 * copied at the head
 * If there isn't enough space, n(availableEndSpace) amount of bytes will be 
 * copied in slice 1.
 * 
 * 5. Copy at the start of buffer, the remaining header message bytes (0-2)
 * - Slice 2: Only if there wasn't enough available space, n(2-availableEndSpace)
 * remaning msg bytes will be copied at start of buffer.
 * 
 * 5. Advance the head index 
 * - The % ensures head index wraps around
 * 
 * -----------------WRITING PAYLOAD-----------------
 * 1. Convert void data pointer into byte pointer
 * payloadBytes pointer, points to array of bytes
 * 
 * 2. Recalculate available endSpace
 * 
 * 3. Limit number of bytes to send to size of payload 
 * - If the number of bytes between head and end of buffer is greater than 
 * the number of bytes in the payload, then set the availableEndSpace = payload size
 * 
 * 4. Copy at the head, availableEndSpace amount of bytes of the payload message (0-size)
 * - At the head, copy availableEndSpace number of bytes from the payLoadBytes 
 * pointer. e.g if we have lots of end space, the avEndSpace is set the size of 
 * payload therefore, we copy all to head. 
 * - If the availableEndSpace is less than the size of the payload, the value is
 * already set at less than the size of the payload, so will only set that amount
 * of bytes from the payload bytes pointer at the head.
 * 
 * 5. Copy at the start of buffer, the remaining payload bytes (0-size)
 * - If availableEndSpace is larger than payload, the number of bytes to be 
 * copied will be sizeofpayload - availablespace = 0, so nothing is copied
 * - If the availableEndSpace is smaller, we add that amount of remaining byes 
 * at the start of the buffer and we offset the payloadbytes pointer 
 * 
 * 6. Advance the head index again 
 * - The % ensures head index wraps around
 * 
 * 7. Update occupied bytes in queue (header+payload)
 * 
 * 8. Return success message



 * @param[in] q     Queue handle.
 * @param[in] data  Pointer to bytes to store (may be NULL only if size==0).
 * @param[in] size  Number of bytes to store.
 *
 * @return QUEUE_OK on success, QUEUE_ERR_FULL if not enough space.
 */
QueueStatus Queue_Send(Queue *q, const void *data, size_t size);

/**
 * @brief Read (dequeue) the next message payload.
 *
 * @param[in]     q          Queue handle.
 * @param[out]    out        Destination buffer (may be NULL only if out_cap==0).
 * @param[in]     out_cap    Capacity of destination buffer in bytes.
 * @param[out]    out_size   Receives actual message length (can be used even if buffer too small).
 *
 * @return
 *  - QUEUE_OK if a message was read and removed from queue.
 *  - QUEUE_ERR_EMPTY if queue is empty.
 *  - QUEUE_ERR_BUFFER_TOO_SMALL if the next message length > out_cap (message is NOT removed).
 */
QueueStatus Queue_Read(Queue *q, void *out, size_t out_cap, size_t *out_size);

/**
 * @brief Destroy/close queue and free resources.
 *
 * @param[in] q Queue handle (may be NULL).
 */
void Queue_Close(Queue *q);

/**
 * @brief Helper function to print initial queue object buffer
 * 
 * @param[in] q Queue instance
 */
void Queue_Debug_PrintInitialBuffer(const Queue *q);

void Queue_DebugPrintState(const Queue *q);

void Queue_Debug_TestSend(Queue *q, const void *data, size_t len);

void Queue_Debug_TestRead(Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */