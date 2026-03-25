/* Dominic Spence - 23/03/2026
 *
 * Description:
 * Implementation of 2 drivers:
 * 1. Queue Driver: A circular byte queue used to store variable length
 * messages. Each message is stored with a length header followed by
 * the payload. [length header (2bytes)][payload (variable length)]
 * 
 * 2. Comms Driver: A communication port emulator that uses the queue driver 
 * internally to send/receive messages. Each message payload structured as:
 * [command (2 bytes)][length(2 bytes)][data payload (variable length)]
 * 
 * 
 * Work Completed:
 * Queue Driver:
 * - Created a queue driver that sends and receives variable length payload
 * messages. 
 * - Operates as a circular buffer that:
 *      - Fails when queue is full
 *      - Fails when read buffer is too small 
 * - Added helper functions to view buffer contents while testing code
 * 
 * Comm Driver:
 * - Created the comm driver to send & receieve messages using 2 internal 
 * queue drivers tx and rx.
 * - Implemented rx emulation to simulate receiving data from hardware
 * - BONUS: Added rx interupt that triggers callback function whenever messages
 * are receieved by rx.
 *
 * - Created testing document to demonstrate testing edge cases
 * - Updated header files to provide @details for each functions
 * 
 *
 * Future Work / Improvements:
 * - Add peek length first pattern on queueRead when read buffer is too small
 */
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