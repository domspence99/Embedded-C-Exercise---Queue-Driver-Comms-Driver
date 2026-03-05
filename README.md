# Embedded C Interview Exercise — Queue Driver + Comms Driver (with optional RX interrupt callback)

## Goal
Implement two small C “drivers”:

1. **Queue driver**: a dynamically allocated message queue that stores variable-length payloads.
2. **Comms driver (emulated)**: a communication port emulator that uses the queue driver internally to send/receive messages.

You must provide:
- Header files (public APIs) with the required structures.
- Documentation comments (Doxygen-style preferred).
- A `main.c` showing example usage.
- (Optional) RX “interrupt” callback using a function pointer.

---

## Project layout (suggested)

/src
queue.c
comm.c
main.c
/include
queue.h
comm.h
/README.md


---

## Requirements

### General
- Language: **C (C99)**.
- No OS dependencies (portable C).
- Must compile cleanly with `-Wall -Wextra -Werror`.
- No global mutable state (unless you justify it clearly).
- Clear error handling (return codes).

### Queue driver requirements
A queue object must be created with a **capacity** (in bytes or in “slots” — you choose, but document it clearly). It must support:
- `init` with dynamic memory allocation
- `send` (enqueue) using **pointer to data** + **size**
- `read` (dequeue) to a user buffer
- `close` (free resources)

It must store messages as:
- `length` + `payload bytes`

**Decide and document**:
- Behavior when queue is full: fail with error or overwrite oldest (pick one).
- Behavior when read buffer is too small: fail, or allow “peek length first” pattern (recommended).
- Thread-safety: not required (single-thread assumption is fine), but keep code clean.

### Comms driver requirements
Implements an emulated port:
- `init comm port`
- `send data`
- `receive data`
- `close comm port`

Internally uses the queue driver for RX/TX (your choice: one queue or two queues, but document it).

You must define a **message structure** for comms:
- `command` (e.g., `uint16_t`)
- `length` (e.g., `uint16_t`)
- `data pointer` (e.g., `const uint8_t*`)

### Optional (bonus)
Implement a callback function pointer that acts like an “RX interrupt”:
- When new RX data arrives into the comm driver, invoke:
  - `on_rx(void *user_ctx)` or
  - `on_rx(const CommMsgHeader* hdr, void *user_ctx)` (your choice)

Document exactly when the callback is called.

---

## Public API (headers to implement)

### `include/queue.h`
```c
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
 * @param[out] q         Pointer that will receive allocated queue handle.
 * @param[in]  capacity  Capacity in bytes for internal storage.
 *
 * @return QUEUE_OK on success.
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

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */

include/comm.h

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

#ifdef __cplusplus
}
#endif

#endif /* COMM_H */
```
What you must implement
1) src/queue.c

Implement the queue as a ring buffer in bytes, storing messages:

[uint16_t len][len bytes payload][uint16_t len][payload]...

Notes:

Use dynamic allocation in Queue_Init.

Return QUEUE_ERR_FULL if there is not enough contiguous/logical free space.

For simplicity, you may implement a ring with “wrap handling” by copying in two parts.

2) src/comm.c

Implement comm emulation:

Maintain internal TX and RX queues (Queue* tx; Queue* rx;)

Comm_Send() serializes header+payload into TX queue.

Comm_EmulateRx() serializes into RX queue and triggers callback if present.

Comm_Receive() reads from RX queue and deserializes.

Example program (src/main.c)

Create this file and make sure it runs and prints something meaningful.

```c
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "comm.h"

static void on_rx_irq(void *user_ctx)
{
    const char *name = (const char *)user_ctx;
    printf("[CB] RX interrupt for %s!\n", name);
}

int main(void)
{
    Comm *comm = NULL;

    if (Comm_Init(&comm, 256, 256) != COMM_OK) {
        printf("Comm_Init failed\n");
        return 1;
    }

    /* Optional callback */
    (void)Comm_SetRxCallback(comm, on_rx_irq, (void*)"COMM0");

    /* Send a message (goes to TX queue) */
    const uint8_t tx_payload[] = {0x10, 0x20, 0x30};
    CommMsg tx_msg = {
        .command = 0x1234,
        .length  = (uint16_t)sizeof(tx_payload),
        .data    = tx_payload
    };

    if (Comm_Send(comm, &tx_msg) != COMM_OK) {
        printf("Comm_Send failed\n");
        Comm_Close(comm);
        return 1;
    }
    printf("Sent message cmd=0x%04X len=%u\n", tx_msg.command, tx_msg.length);

    /* Emulate RX arrival (as if hardware received it) */
    const char *hello = "hello";
    CommMsg rx_in = {
        .command = 0x0042,
        .length  = (uint16_t)strlen(hello),
        .data    = (const uint8_t*)hello
    };

    if (Comm_EmulateRx(comm, &rx_in) != COMM_OK) {
        printf("Comm_EmulateRx failed\n");
        Comm_Close(comm);
        return 1;
    }

    /* Receive it */
    uint16_t cmd = 0;
    uint8_t  buf[32];
    size_t   len = 0;

    CommStatus st = Comm_Receive(comm, &cmd, buf, sizeof(buf), &len);
    if (st == COMM_OK) {
        printf("Received cmd=0x%04X len=%zu data='", cmd, len);
        for (size_t i = 0; i < len; i++) putchar((char)buf[i]);
        printf("'\n");
    } else if (st == COMM_ERR_EMPTY) {
        printf("No RX data\n");
    } else {
        printf("Comm_Receive error=%d\n", (int)st);
    }

    Comm_Close(comm);
    return 0;
}
```
Acceptance criteria checklist

 Queue_Init/Send/Read/Close implemented and documented.

 Queue stores variable-length messages correctly.

 Comm_Init/Send/Receive/Close implemented and documented.

 Comm_EmulateRx works and is used in main.c.

 main.c demonstrates send + emulate RX + receive.

 Optional callback triggers on emulate RX (if implemented).

 Clean compilation with warnings enabled.

Notes / Hints (you may include or ignore)

Prefer fixed-width integers (stdint.h).

Keep APIs robust: validate null pointers and size rules.

If you choose uint16_t for lengths, document max payload size (65535).

For the queue ring buffer, you’ll need a way to detect full/empty and to manage wrap-around.

Optional extensions (bonus points)

Peek function to read next message length without removing.

Unit tests (even minimal).

Support zero-length payload messages.

Good luck!

