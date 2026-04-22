#include "cross_section_manager.h"
#include "packet.h"

Line s_plant_line_objects[CROSS_SECTION_MAX_LINES];
Line s_model_line_objects[CROSS_SECTION_MAX_LINES];
size_t s_num_plant_lines = 0;
size_t s_num_model_lines = 0;
size_t s_current_slice_idx = 0;

void cross_section_manager_give_packet(PacketID id, const uint8_t* data,
                                       int data_length) {
    switch (id.type) {
        case ((PacketID)PACKET_TYPE_CROSS_SECTION).type: {
            if (data_length < 5 ||
                (data_length - 5) % (4 * sizeof(float)) != 0) {
                TRACE_PRINTF(
                    "Received cross section packet with invalid length!\n");
                return;
            }

            size_t num_lines = (data_length - 5) / (4 * sizeof(float));

            // First byte: [7:1] slice idx, [0] is_model_segment
            uint8_t slice_idx = data[0] >> 1;
            bool is_model_segment = (data[0] & 0x1) != 0;

            // Next 4 bytes: float representing the "radius" of the square
            // slices
            float slice_radius = *((float*)(data + 1));

            printf(
                "Received cross section packet for slice idx: %u, "
                "is_model_segment: "
                "%d, num_lines: %lu, slice_radius: %f\n",
                slice_idx, is_model_segment, (uint32_t)num_lines, slice_radius);

            // If this is a new slice reset the buffers
            if (slice_idx != s_current_slice_idx) {
                s_current_slice_idx = slice_idx;
                s_num_plant_lines = 0;
                s_num_model_lines = 0;

                for (size_t i = 0; i < CROSS_SECTION_MAX_LINES; i++) {
                    s_plant_line_objects[i].set_visible(false);
                    s_plant_line_objects[i].set_color(0xF3);
                    s_model_line_objects[i].set_visible(false);
                    s_model_line_objects[i].set_color(0xF2);
                }
            }

            Line* dest_arr =
                is_model_segment ? s_model_line_objects : s_plant_line_objects;
            size_t& num_lines_in_dest =
                is_model_segment ? s_num_model_lines : s_num_plant_lines;

            // Take as many lines as we can fit and buffer them
            for (size_t i = 0;
                 i < num_lines && num_lines_in_dest < CROSS_SECTION_MAX_LINES;
                 i++) {
                float x1 = *((float*)(data + 5 + i * 16 + 0));
                float y1 = *((float*)(data + 5 + i * 16 + 4));
                float x2 = *((float*)(data + 5 + i * 16 + 8));
                float y2 = *((float*)(data + 5 + i * 16 + 12));

                printf("Received line segment: (%f, %f) to (%f, %f)\n", x1, y1,
                       x2, y2);

                // Now scale and translate the line segments to pixel coords
                // The center of the window is at 560, 163
                constexpr unsigned int SLICER_WINDOW_CENTER_X = 560;
                constexpr unsigned int SLICER_WINDOW_CENTER_Y = 163;
                // The window is 320, 320
                constexpr unsigned int SLICER_WINDOW_WIDTH = 320;
                constexpr unsigned int SLICER_WINDOW_HEIGHT = 320;
                // Points at (-slice_radius, -slice_radius) should map to the
                // bottom left of the window, and points at (slice_radius,
                // slice_radius) should map to the top right
                float scale_x = SLICER_WINDOW_WIDTH / (2 * slice_radius);
                float scale_y = SLICER_WINDOW_HEIGHT / (2 * slice_radius);
                unsigned int x1_px =
                    SLICER_WINDOW_CENTER_X + (int)(x1 * scale_x);
                unsigned int y1_px =
                    SLICER_WINDOW_CENTER_Y + (int)(y1 * scale_y);
                unsigned int x2_px =
                    SLICER_WINDOW_CENTER_X + (int)(x2 * scale_x);
                unsigned int y2_px =
                    SLICER_WINDOW_CENTER_Y + (int)(y2 * scale_y);

                printf(
                    "Scaled line segment to pixel coords: (%u, %u) to (%u, "
                    "%u)\n",
                    x1_px, y1_px, x2_px, y2_px);

                // Clamp to window bounds
                if (x1_px > SLICER_WINDOW_CENTER_X + SLICER_WINDOW_WIDTH / 2) {
                    x1_px = SLICER_WINDOW_CENTER_X + SLICER_WINDOW_WIDTH / 2;
                }

                if (y1_px > SLICER_WINDOW_CENTER_Y + SLICER_WINDOW_HEIGHT / 2) {
                    y1_px = SLICER_WINDOW_CENTER_Y + SLICER_WINDOW_HEIGHT / 2;
                }

                if (x1_px < SLICER_WINDOW_CENTER_X - SLICER_WINDOW_WIDTH / 2) {
                    x1_px = SLICER_WINDOW_CENTER_X - SLICER_WINDOW_WIDTH / 2;
                }

                if (y1_px < SLICER_WINDOW_CENTER_Y - SLICER_WINDOW_HEIGHT / 2) {
                    y1_px = SLICER_WINDOW_CENTER_Y - SLICER_WINDOW_HEIGHT / 2;
                }

                printf(
                    "Clamped line segment to pixel coords: (%u, %u) to (%u, "
                    "%u)\n",
                    x1_px, y1_px, x2_px, y2_px);

                Line* line = &dest_arr[num_lines_in_dest];

                line->set_visible(true);
                line->set_start(x1_px, y1_px);
                line->set_end(x2_px, y2_px);

                num_lines_in_dest++;
            }

            // Send an ACK
            PacketID id = PACKET_TYPE_CROSS_SECTION;
            id.ack = 1;
            packet_send(NULL, 0, id);

            break;
        }

        default:
            break;
    }
}

void cross_section_manager_get_layer(uint8_t slice_idx) {
    PacketID id = PACKET_TYPE_GET_CROSS_SECTION;
    uint8_t data[1] = {slice_idx};
    packet_send(data, sizeof(data), id);
}

void cross_section_manager_create_cross_sections(uint8_t num_slices) {
    PacketID id = PACKET_TYPE_CREATE_CROSS_SECTIONS;
    uint8_t data[1] = {num_slices};
    packet_send(data, sizeof(data), id);
}

size_t cross_section_manager_get_num_plant_lines() {
    return CROSS_SECTION_MAX_LINES;
}

size_t cross_section_manager_get_num_model_lines() {
    return CROSS_SECTION_MAX_LINES;
}

Line* cross_section_manager_get_plant_lines() {
    return s_plant_line_objects;
}

Line* cross_section_manager_get_model_lines() {
    return s_model_line_objects;
}