#pragma once

#include "packet.h"
#include "status.h"
#include "gui.h"

#define CROSS_SECTION_MAX_LINES 128

void cross_section_receiver_give_packet(PacketID id, const uint8_t* data,
                                        int data_length);
size_t cross_section_receiver_get_num_plant_lines();
Line* cross_section_receiver_get_plant_lines();
size_t cross_section_receiver_get_num_model_lines();
Line* cross_section_receiver_get_model_lines();