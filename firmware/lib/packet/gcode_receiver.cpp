#include "gcode_receiver.h"

#include "status.h"
#include "gcode/queue.h"
#include "timing.h"
#include "stm32h7xx_hal.h"

#include <stdio.h>
#include <string.h>

#define GCODE_SEMAPHORE_MAX_WAIT 1000

__attribute__((used))
__attribute__((section(".gcode_receiver_ctx"))) static struct {
    char s_gcode_buf[MAX_CMD_SIZE+1];   // Buffer for the command yet to be acked
    bool s_gcode_buf_in_waiting = false;
} gcode_receiver_ctx;

bool s_ack_waiting = false;

void gcode_receiver_task() {
#ifdef CORE_CM7

    // ACK a packet if it's waiting

    // TAKE LOCK
    uint32_t start_tick = HAL_GetTick();
    while (HAL_HSEM_Take(1, 0) != HAL_OK) {
        if (HAL_GetTick() - start_tick > GCODE_SEMAPHORE_MAX_WAIT) {
            TRACE_PRINTF("G-code receiver semaphore timeout: BAD\n");
            return;  // timeout
        }
    }

    if (s_ack_waiting && !gcode_receiver_ctx.s_gcode_buf_in_waiting) {
        // If we were waiting to ack but now the M4 took the packet, we can ack it now
        PacketID ack_id = PACKET_TYPE_GCODE;
        ack_id.ack = 1;
        int res = packet_send(nullptr, 0, ack_id);

        if (res >= 0) {
            s_ack_waiting = false;
        } else {
            TRACE_PRINTF("Failed to send ACK for G-code command: %s\n", gcode_receiver_ctx.s_gcode_buf);
        }
    }

    // RELEASE LOCK
    HAL_HSEM_Release(1, 0);

#else
    // TAKE LOCK
    uint32_t start_tick = HAL_GetTick();
    while (HAL_HSEM_Take(1, 0) != HAL_OK) {
        if (HAL_GetTick() - start_tick > GCODE_SEMAPHORE_MAX_WAIT) {
            TRACE_PRINTF("G-code receiver semaphore timeout: BAD\n");
            return;  // timeout
        }
    }

    if (gcode_receiver_ctx.s_gcode_buf_in_waiting) {
        if (queue.ring_buffer.full()) {
            goto release;
        }

        // Enqueue it
        if (queue.enqueue_one(gcode_receiver_ctx.s_gcode_buf) != true) {
            TRACE_PRINTF("Failed to enqueue G-code command: %s\n", gcode_receiver_ctx.s_gcode_buf);
            goto release;
        }

        printf("Enqueued G-code: %s\n", gcode_receiver_ctx.s_gcode_buf);

        gcode_receiver_ctx.s_gcode_buf_in_waiting = false;
    }

    release:

    // RELEASE LOCK
    HAL_HSEM_Release(1, 0);
#endif
}

void gcode_receiver_give_packet(PacketID id, const uint8_t* data,
                                int data_length) {
    if (id.type == ((PacketID)PACKET_TYPE_GCODE).type) {
        // TAKE LOCK
        uint32_t start_tick = HAL_GetTick();
        while (HAL_HSEM_Take(1, 0) != HAL_OK) {
            if (HAL_GetTick() - start_tick > GCODE_SEMAPHORE_MAX_WAIT) {
                TRACE_PRINTF("G-code receiver semaphore timeout: BAD\n");
                return;  // timeout
            }
        }

        printf("Received G-code packet: %.*s\n", data_length, data);
        switch(gcode_receiver_ctx.s_gcode_buf_in_waiting) {
            case false:
                // If nothing is waiting, put it in buf and wait to ack
                memcpy(gcode_receiver_ctx.s_gcode_buf, (const char*)data, data_length);
                gcode_receiver_ctx.s_gcode_buf[data_length] = '\0';
                gcode_receiver_ctx.s_gcode_buf_in_waiting = true;

                // So the loop knows to ACK
                s_ack_waiting = true;

                break;
            case true:
                // If one is already waiting, something went very wrong
                TRACE_PRINTF("Received G-code packet while another is waiting to be acked!\n");
                break;
        }

        // release lock
        HAL_HSEM_Release(1, 0);

    }
}