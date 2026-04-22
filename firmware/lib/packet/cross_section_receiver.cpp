#include "cross_section_receiver.h"

Line s_plant_line_objects[CROSS_SECTION_MAX_LINES];
Line s_model_line_objects[CROSS_SECTION_MAX_LINES];
size_t s_num_plant_lines = 0;
size_t s_num_model_lines = 0;
size_t s_current_slice_idx = 0;

void cross_section_receiver_give_packet(PacketID id, const uint8_t* data,
                                        int data_length) {
    if (data_length < 5 || (data_length - 5) % (4 * sizeof(float)) != 0) {
        TRACE_PRINTF("Received cross section packet with invalid length!\n");
        return;
    }

    size_t num_lines = (data_length - 5) / (4 * sizeof(float));

    // First byte: [7:1] slice idx, [0] is_model_segment
    uint8_t slice_idx = data[0] >> 1;
    bool is_model_segment = (data[0] & 0x1) != 0;

    // Next 4 bytes: float representing the "radius" of the square slices
    float slice_radius = *((float*)(data + 1));

    // If this is a new slice reset the buffers
    if (slice_idx != s_current_slice_idx) {
        s_current_slice_idx = slice_idx;
        s_num_plant_lines = 0;
        s_num_model_lines = 0;

        for (size_t i = 0; i < CROSS_SECTION_MAX_LINES; i++) {
            s_plant_line_objects[i].set_visible(false);
            s_model_line_objects[i].set_visible(false);
        }
    }

    Line* dest_arr =
        is_model_segment ? s_model_line_objects : s_plant_line_objects;
    size_t& num_lines_in_dest =
        is_model_segment ? s_num_model_lines : s_num_plant_lines;

    // Take as many lines as we can fit and buffer them
    for (size_t i = 0;
         i < num_lines && num_lines_in_dest < CROSS_SECTION_MAX_LINES; i++) {
        float x1 = *((float*)(data + 5 + i * 4 + 0));
        float y1 = *((float*)(data + 5 + i * 4 + 4));
        float x2 = *((float*)(data + 5 + i * 4 + 8));
        float y2 = *((float*)(data + 5 + i * 4 + 12));

        // Now scale and translate the line segments to pixel coords
        // The center of the window is at 560, 163
        constexpr unsigned int SLICER_WINDOW_CENTER_X = 560;
        constexpr unsigned int SLICER_WINDOW_CENTER_Y = 163;
        // The window is 320, 320
        constexpr unsigned int SLICER_WINDOW_WIDTH = 320;
        constexpr unsigned int SLICER_WINDOW_HEIGHT = 320;
        // Points at (-slice_radius, -slice_radius) should map to the bottom
        // left of the window, and points at (slice_radius, slice_radius) should
        // map to the top right
        float scale_x = SLICER_WINDOW_WIDTH / (2 * slice_radius);
        float scale_y = SLICER_WINDOW_HEIGHT / (2 * slice_radius);
        unsigned int x1_px = SLICER_WINDOW_CENTER_X +
                             (unsigned int)((x1 / slice_radius) * scale_x);
        unsigned int y1_px = SLICER_WINDOW_CENTER_Y +
                             (unsigned int)((y1 / slice_radius) * scale_y);
        unsigned int x2_px = SLICER_WINDOW_CENTER_X +
                             (unsigned int)((x2 / slice_radius) * scale_x);
        unsigned int y2_px = SLICER_WINDOW_CENTER_Y +
                             (unsigned int)((y2 / slice_radius) * scale_y);

        // Clamp to window bounds
        if (x1_px > SLICER_WINDOW_CENTER_X + SLICER_WINDOW_WIDTH) {
            x1_px = SLICER_WINDOW_CENTER_X + SLICER_WINDOW_WIDTH;
        }

        if (y1_px > SLICER_WINDOW_CENTER_Y + SLICER_WINDOW_HEIGHT) {
            y1_px = SLICER_WINDOW_CENTER_Y + SLICER_WINDOW_HEIGHT;
        }

        if (x1_px < SLICER_WINDOW_CENTER_X - SLICER_WINDOW_WIDTH) {
            x1_px = SLICER_WINDOW_CENTER_X - SLICER_WINDOW_WIDTH;
        }

        if (y1_px < SLICER_WINDOW_CENTER_Y - SLICER_WINDOW_HEIGHT) {
            y1_px = SLICER_WINDOW_CENTER_Y - SLICER_WINDOW_HEIGHT;
        }

        Line* line = &dest_arr[num_lines_in_dest];

        line->set_visible(true);
        line->set_start(x1_px, y1_px);
        line->set_end(x2_px, y2_px);

        num_lines_in_dest++;
    }
}

size_t cross_section_receiver_get_num_plant_lines() {
    return CROSS_SECTION_MAX_LINES;
}

size_t cross_section_receiver_get_num_model_lines() {
    return CROSS_SECTION_MAX_LINES;
}

Line* cross_section_receiver_get_plant_lines() {
    return s_plant_line_objects;
}

Line* cross_section_receiver_get_model_lines() {
    return s_model_line_objects;
}