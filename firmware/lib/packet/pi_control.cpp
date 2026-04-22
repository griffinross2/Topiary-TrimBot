#include "pi_control.h"

#include "timing.h"
#include "gcode/queue.h"
#include "module/planner.h"

__attribute__((used))
__attribute__((section(".pi_control_ctx"))) static struct {
    bool moving = false;
} s_pi_control_ctx;

static uint32_t s_last_status_update_time = 0;
static bool s_scanning = false;

void pi_control_task() {
#ifdef CORE_CM7
    if (get_tick_ms() - s_last_status_update_time >=
        PI_STATUS_UPDATE_INTERVAL_MS) {
        uint8_t status = s_pi_control_ctx.moving ? 1 : 0;

        packet_send(&status, sizeof(status), PACKET_TYPE_STATUS);

        s_last_status_update_time = get_tick_ms();
    }
#else
    s_pi_control_ctx.moving = queue.ring_buffer.length > 0 || planner.busy();
#endif
}

Status pi_control_start_scanning() {
    PacketID id = PACKET_TYPE_START_SCANNING;
    int res = packet_send(nullptr, 0, id);

    if (res < 0) {
        return STATUS_ERROR;
    }

    s_scanning = true;

    return STATUS_OK;
}

void pi_control_give_packet(PacketID id, const uint8_t* data, int data_length) {
    switch (id.type) {
        case ((PacketID)PACKET_TYPE_DONE_SCANNING).type:
            s_scanning = false;
            break;

        default:
            break;
    }
}

bool pi_control_is_scanning() {
    return s_scanning;
}