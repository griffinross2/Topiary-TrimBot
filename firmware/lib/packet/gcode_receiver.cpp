#include "gcode_receiver.h"

#include "status.h"
#include "gcode/queue.h"

#include <stdio.h>
#include <string.h>

static char __attribute__((section(".gcode_buffer"))) s_gcode_buf[256];
static bool __attribute__((section(".gcode_buffer"))) s_gcode_buf_flag = false;

void gcode_receiver_task() {
#ifdef CORE_CM7
    return;
#endif

    if (s_gcode_buf_flag) {
        printf("%d\n", queue.ring_buffer.length);
        if (queue.ring_buffer.full()) {
            return;
        }

        if (queue.enqueue_one(s_gcode_buf) != true) {
            return;
        }

        printf("Enqueued G-code: %s\n", s_gcode_buf);

        // Clear the flag to indicate we are ready for the next G-code
        s_gcode_buf_flag = false;
    }
}

void gcode_receiver_give_packet(PacketID id, const uint8_t* data,
                                int data_length) {
    if (id.type == ((PacketID)PACKET_TYPE_GCODE).type) {
        printf("Received G-code packet: %.*s\n", data_length, data);

        if (s_gcode_buf_flag) {
            // The gcode in the buffer hasn't been processed yet, so we should
            // not ACK this packet
        } else {
            // ACK first and make sure it is successful
            PacketID ack_id = PACKET_TYPE_GCODE;
            ack_id.ack = 1;
            int res = packet_send(nullptr, 0, ack_id);

            if (res < 0) {
                // Stop here to avoid us committing the same G-code twice
                TRACE_PRINTF("Failed to send ACK for G-code packet!\n");
                return;
            }

            // Copy the gcode to the buffer and set the flag for the other core
            // to process
            memcpy(s_gcode_buf, data, data_length);
            s_gcode_buf[data_length] = '\0';
            s_gcode_buf_flag = true;
        }
    }
}