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

void Queue_DebugPrintBuffer(const Queue *q);

void Queue_DebugPrintState(const Queue *q);

void Queue_Debug_TestSend(Queue *q, const void *data, size_t len);

void Queue_Debug_TestRead(Queue *q);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */