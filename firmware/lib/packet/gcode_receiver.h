#pragma once

#include "packet.h"

void gcode_receiver_task();
void gcode_receiver_give_packet(PacketID id, const uint8_t* data, int data_length);