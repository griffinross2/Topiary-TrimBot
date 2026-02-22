#include "packet.h"

#include "crc16.h"
#include "status.h"
#include "usb.h"
#include "cobs.h"

#include <string.h>

int packet_fill(uint8_t* buf, int data_length, PacketID id) {
    PacketHeader header;
    header.id = id;
    header.length = data_length;

    // Copy header to destination
    memcpy(buf, &header, sizeof(PacketHeader));

    // Calculate CRC and copy to destination
    PacketCRC crc = crc16_ccitt(buf, sizeof(PacketHeader) + data_length);
    memcpy(buf + sizeof(PacketHeader) + data_length, &crc, sizeof(PacketCRC));

    return PACKET_SIZE(data_length);
}

int packet_check(const uint8_t* buf, int total_length) {
    if (total_length < PACKET_HEADER_SIZE + sizeof(PacketCRC)) {
        // Not enough data for header and CRC
        return -1;
    }

    const PacketHeader* header = (const PacketHeader*)buf;

    if (total_length != PACKET_SIZE(header->length)) {
        // Total length does not match expected packet size
        return -1;
    }

    // Calculate CRC
    PacketCRC calculated_crc =
        crc16_ccitt(buf, sizeof(PacketHeader) + header->length);

    // Get actual received CRC
    PacketCRC received_crc;
    memcpy(&received_crc, buf + sizeof(PacketHeader) + header->length,
           sizeof(PacketCRC));

    if (calculated_crc != received_crc) {
        // CRC mismatch
        TRACE_PRINTF("CRC mismatch! Calculated: %04X, Received: %04X\n",
                     calculated_crc, received_crc);

        return -1;
    }

    return 0;
}

int packet_send(const uint8_t* data, int data_length, PacketID id) {
    if (data_length > MAX_PACKET_DATA_SIZE) {
        // Too much data
        return -1;
    }

    uint8_t buf[PACKET_SIZE(MAX_PACKET_DATA_SIZE)];
    memcpy(buf + sizeof(PacketHeader), data, data_length);
    int packet_len = packet_fill(buf, data_length, id);
    if (packet_len < 0) {
        return -1;
    }

    // COBS encode
    uint8_t encoded_buf[PACKET_SIZE(MAX_PACKET_DATA_SIZE) + 2];
    cobs_encode_result res =
        cobs_encode(encoded_buf, sizeof(encoded_buf), buf, packet_len);
    if (res.out_len < 0) {
        return -1;
    }

    // Add delimiter
    encoded_buf[res.out_len] = 0x0;

    // Send over USB
    return usb_send((char*)encoded_buf, res.out_len + 1);
}

bool packet_can_send(int data_length) {
    if (data_length > MAX_PACKET_DATA_SIZE) {
        return false;
    }

    int packet_len = PACKET_SIZE(data_length);
    int encoded_len = packet_len + 1;  // COBS

    return usb_available_write() >= encoded_len + 1;  // +1 for delimiter
}