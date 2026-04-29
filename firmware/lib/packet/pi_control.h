#pragma once

#include "packet.h"
#include "status.h"

#define PI_STATUS_UPDATE_INTERVAL_MS 1000

void pi_control_task();
Status pi_control_start_scanning();
Status pi_control_start_toolpathing();
Status pi_control_start_cutting();
void pi_control_give_packet(PacketID id, const uint8_t* data, int data_length);
bool pi_control_is_scanning();
bool pi_control_is_toolpathing();
bool pi_control_is_cutting();
bool pi_control_is_booted();