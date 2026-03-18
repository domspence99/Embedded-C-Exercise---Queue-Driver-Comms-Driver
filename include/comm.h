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
 * If you implement this, call it when RX receives new bytes/message.
 */
CommStatus Comm_SetRxCallback(Comm *comm, CommRxCallback cb, void *user_ctx);

/**
 * @brief Send a message.
 *
 * The driver must serialize:
 *   [command(uint16_t)][length(uint16_t)][payload bytes]
 * into its TX path (queue).
 */
CommStatus Comm_Send(Comm *comm, const CommMsg *msg);

/**
 * @brief Receive a message.
 *
 * Recommended pattern:
 * - Caller provides a payload buffer and capacity.
 * - Driver fills out_cmd/out_len and copies payload into buffer.
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
 * This function simulates that bytes arrived from "hardware" into RX.
 * It should push the serialized message into the RX queue and trigger
 * the RX callback (if implemented).
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