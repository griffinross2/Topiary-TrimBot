#pragma once

#include "packet.h"
#include "status.h"
#include "gui.h"

#define CROSS_SECTION_MAX_LINES 64
#define CROSS_SECTION_NUM_SLICES 16

void cross_section_manager_give_packet(PacketID id, const uint8_t* data,
                                       int data_length);
void cross_section_manager_get_layer(uint8_t slice_idx);
void cross_section_manager_create_cross_sections(uint8_t num_slices);

size_t cross_section_manager_get_num_plant_lines();
Line* cross_section_manager_get_plant_lines();
size_t cross_section_manager_get_num_model_lines();
Line* cross_section_manager_get_model_lines();