#include "comm.h"

CommStatus Comm_Init(Comm **comm, size_t tx_capacity, size_t rx_capacity)
{
    (void)comm;
    (void)tx_capacity;
    (void)rx_capacity;
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
    (void)comm;
    (void)msg;
    return COMM_OK;
}

CommStatus Comm_Receive(Comm *comm,
                        uint16_t *out_cmd,
                        uint8_t *out_data,
                        size_t out_cap,
                        size_t *out_len)
{
    (void)comm;
    (void)out_cmd;
    (void)out_data;
    (void)out_cap;
    (void)out_len;

    return COMM_ERR_EMPTY;
}

CommStatus Comm_EmulateRx(Comm *comm, const CommMsg *msg)
{
    (void)comm;
    (void)msg;
    return COMM_OK;
}

void Comm_Close(Comm *comm)
{
    (void)comm;
}
