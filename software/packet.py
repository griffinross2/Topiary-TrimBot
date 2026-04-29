import cobs.cobs
from crc16 import crc16_ccitt
from dataclasses import dataclass
from usb import USBDev

# Utility functions
def packet_size(data_len):
    return 2 + data_len + 2

def packet_header_size():
    return 2
    
def max_packet_data_size():
    return 254 - 2 - 2

@dataclass
class PacketID:
    packet_type: int
    ack_req: bool
    ack: bool

    def to_bytes(self) -> bytes:
        return bytes([(((1 if self.ack else 0) << 7) + ((1 if self.ack_req else 0) << 6) + (self.packet_type & 0x3F))])

    @classmethod
    def from_bytes(cls, data: int):
        return cls(packet_type=(data & 0x3F), ack_req=(data & 0x40 == 0x40), ack=(data & 0x80 == 0x80))

@dataclass
class PacketHeader:
    id: PacketID
    length: int

    def to_bytes(self) -> bytes:
        return self.id.to_bytes() + bytes([self.length])
    
    @classmethod
    def from_bytes(cls, data: bytes):
        return cls(id=PacketID.from_bytes(data[0]), length=data[1])

PACKET_TYPE_FILE_START = PacketID(0x00, 1, 0)
PACKET_TYPE_FILE_CHUNK = PacketID(0x01, 0, 0)
PACKET_TYPE_FILE_RESEND = PacketID(0x02, 0, 0)
PACKET_TYPE_FILE_END = PacketID(0x03, 1, 0)
PACKET_TYPE_GCODE = PacketID(0x10, 1, 0)
PACKET_TYPE_STATUS = PacketID(0x20, 0, 0)
PACKET_TYPE_START_SCANNING = PacketID(0x21, 0, 0)
PACKET_TYPE_DONE_SCANNING = PacketID(0x22, 0, 0)
PACKET_TYPE_START_TOOLPATHING = PacketID(0x23, 0, 0)
PACKET_TYPE_DONE_TOOLPATHING = PacketID(0x24, 0, 0)
PACKET_TYPE_START_CUTTING = PacketID(0x25, 0, 0)
PACKET_TYPE_DONE_CUTTING = PacketID(0x26, 0, 0)
PACKET_TYPE_PI_BOOTED = PacketID(0x27, 0, 0)
PACKET_TYPE_CROSS_SECTION = PacketID(0x30, 0, 0)
PACKET_TYPE_CREATE_CROSS_SECTIONS = PacketID(0x31, 0, 0)
PACKET_TYPE_GET_CROSS_SECTION = PacketID(0x32, 0, 0)

def packet_fill(data: bytes, id: PacketID):
    header = PacketHeader(id, len(data))
    packet = header.to_bytes()

    packet += data

    crc = crc16_ccitt(packet)
    packet += bytes([crc & 0xFF, (crc >> 8) & 0xFF])

    return packet

def packet_check(packet: bytes):
    if (len(packet) < packet_header_size() + 2):
        return -1

    header = PacketHeader.from_bytes(packet[:2])
    if len(packet) != packet_size(header.length):
        print(f"Packet length {len(packet)} does not match expected {packet_size(header.length)}!")
        return -2
    
    header_plus_data_len = 2 + header.length
    calculated_crc = crc16_ccitt(packet[:header_plus_data_len])
    received_crc = packet[header_plus_data_len] + (packet[header_plus_data_len+1] << 8)

    if (calculated_crc != received_crc):
        print(f"CRC mismatch! Calculated: {calculated_crc:04x}, Received: {received_crc:04x}")
        return -3
    
    return 0

def packet_send(dev: USBDev, data: bytes, id: PacketID):
    if (len(data) > max_packet_data_size()):
        return -1
    
    packet = packet_fill(data, id)

    packet_enc = cobs.cobs.encode(packet)
    packet_enc += bytes([0x00])

    print(f"Sending packet type: {id.packet_type:d}, len: {len(data):d}")

    res = dev.send(packet_enc)

    if res >= 0:
        return res