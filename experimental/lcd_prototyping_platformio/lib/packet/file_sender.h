#pragma once

#include "packet.h"
#include "status.h"

typedef enum {
    FILE_SENDER_STATUS_IDLE,
    FILE_SENDER_STATUS_START,
    FILE_SENDER_STATUS_SENDING,
    FILE_SENDER_STATUS_RESENDING,
    FILE_SENDER_STATUS_END,
    FILE_SENDER_STATUS_ERROR,
} FileSenderStatus;

void file_sender_task();
void file_sender_give_packet(PacketID id, const uint8_t* data, int data_length);

Status file_sender_send_file(const char* filename);