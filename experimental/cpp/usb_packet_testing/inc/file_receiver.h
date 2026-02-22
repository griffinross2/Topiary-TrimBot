#pragma once

#include "packet.h"

typedef enum
{
    FILE_RECEIVER_STATUS_IDLE,
    FILE_RECEIVER_STATUS_START,
    FILE_RECEIVER_STATUS_RECEIVING,
    FILE_RECEIVER_STATUS_RESENDING,
    FILE_RECEIVER_STATUS_END,
    FILE_RECEIVER_STATUS_ERROR,
} FileReceiverStatus;

void file_receiver_task();
void file_receiver_give_packet(PacketID id, const uint8_t *data, int data_length);