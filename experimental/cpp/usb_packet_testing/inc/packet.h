#pragma once

#include <cstdint>

typedef struct
{
    uint8_t type : 6;
    uint8_t ack_req : 1;
    uint8_t ack : 1;
} PacketID;

#define PACKET_TYPE_FILE_START PacketID{0x00, 1, 0}
#define PACKET_TYPE_FILE_CHUNK PacketID{0x01, 0, 0}
#define PACKET_TYPE_FILE_RESEND PacketID{0x02, 0, 0}
#define PACKET_TYPE_FILE_END PacketID{0x03, 1, 0}

static_assert(sizeof(PacketID) == 1, "PacketType should be 1 byte");

typedef struct
{
    PacketID id;
    uint8_t length;
} PacketHeader;

static_assert(sizeof(PacketHeader) == 2, "PacketHeader should be 2 bytes");

typedef uint16_t PacketCRC;

#define PACKET_SIZE(data_len) (sizeof(PacketHeader) + data_len + sizeof(PacketCRC))
#define PACKET_HEADER_SIZE sizeof(PacketHeader)
#define MAX_PACKET_DATA_SIZE (254 - sizeof(PacketHeader) - sizeof(PacketCRC))

int packet_fill(uint8_t *buf, int data_length, PacketID id);
int packet_check(const uint8_t *buf, int total_length);
int packet_send(const uint8_t *data, int data_length, PacketID id);