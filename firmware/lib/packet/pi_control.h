#pragma once

#include "packet.h"
#include "status.h"

#define PI_STATUS_UPDATE_INTERVAL_MS 1000

void pi_control_task();
Status pi_control_start_scanning();
void pi_control_give_packet(PacketID id, const uint8_t* data, int data_length);
bool pi_control_is_scanning();