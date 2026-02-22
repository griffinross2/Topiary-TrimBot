#include "packet_engine.h"

#include "usb.h"
#include "packet.h"
#include "common.h"
#include "cobs.h"
#include "file_receiver.h"

#include <string.h>

static char s_rx_buf[PACKET_ENGINE_BUFFER_SIZE];
static int s_rx_buf_len = 0;

int handle_received_packet(uint8_t *buf, int len)
{
    // COBS decode
    cobs_decode_result res = cobs_decode(buf, len, buf, len);
    len = res.out_len;

    int err = packet_check(buf, len);
    if (err < 0)
    {
        // Invalid packet
        TRACE_PRINTF("Received invalid packet, error: %d\n", err);

        return -1;
    }

    PacketHeader *header = (PacketHeader *)buf;

    TRACE_PRINTF("Received valid packet type: %d, len: %d\n", header->id.type, header->length);
    TRACE_PRINTF("Packet data:");
    for (int i = 0; i < header->length; i++)
    {
        printf("%c", buf[sizeof(PacketHeader) + i]);
    }
    printf("\n");

    // Send off the packet
    switch (header->id.type)
    {
    case PACKET_TYPE_FILE_START.type:
    case PACKET_TYPE_FILE_CHUNK.type:
    case PACKET_TYPE_FILE_RESEND.type:
    case PACKET_TYPE_FILE_END.type:
        file_receiver_give_packet(header->id, buf + sizeof(PacketHeader),
                                  header->length);
        break;

    default:
        break;
    }

    return 0;
}

int packet_engine_init()
{
    return 0;
}

void packet_engine_task()
{
    int len = 0;

    if (usb_available() > 0)
    {
        len = usb_receive(s_rx_buf + s_rx_buf_len, sizeof(s_rx_buf) - s_rx_buf_len);
        s_rx_buf_len += len;
        TRACE_PRINTF("Received %d bytes from USB\n", len);
    }

    // Search through the data for a packet delimiter (0x0)
    for (int i = 0; i < s_rx_buf_len; i++)
    {
        if (s_rx_buf[i] == 0x0)
        {

            // Now send the complete packet to the handler
            handle_received_packet((uint8_t *)s_rx_buf, i);

            // Shift the remaining data to the beginning of the buffer
            int remaining_start = i + 1;
            int remaining_len = s_rx_buf_len - i - 1;
            memmove(s_rx_buf, s_rx_buf + remaining_start, remaining_len);

            // Update the buffer length
            s_rx_buf_len = remaining_len;
        }
    }
}