from usb import USBDev
import cobs.cobs
from packet import *
from file_receiver import file_receiver_give_packet
from gcode_sender import gcode_sender_give_packet

rx_buf = bytes()

def handle_received_packet(packet_enc):
    packet = cobs.cobs.decode(packet_enc)
    
    err = packet_check(packet)
    if err < 0:
        print(f"Received invalid packet, error: {err:d}")
        return -1
    
    header = PacketHeader.from_bytes(packet[:2])

    print(f"Received valid packet type: {header.id.packet_type:d}, len: {header.length}")
    packet_data_start = 2
    packet_data_end = 2 + header.length

    print("Packet data:")
    print(packet[packet_data_start:packet_data_end].decode('utf8', errors="replace"))

    match header.id.packet_type:
        case PACKET_TYPE_FILE_START.packet_type:
            file_receiver_give_packet(header.id, packet[packet_data_start:packet_data_end])
        case PACKET_TYPE_FILE_CHUNK.packet_type:
            file_receiver_give_packet(header.id, packet[packet_data_start:packet_data_end])
        case PACKET_TYPE_FILE_RESEND.packet_type:
            file_receiver_give_packet(header.id, packet[packet_data_start:packet_data_end])
        case PACKET_TYPE_FILE_END.packet_type:
            file_receiver_give_packet(header.id, packet[packet_data_start:packet_data_end])
        case PACKET_TYPE_GCODE.packet_type:
            gcode_sender_give_packet(header.id, packet[packet_data_start:packet_data_end])

        case _:
            pass

    return 0

def packet_engine_task(dev: USBDev):
    global rx_buf
    rx_buf_len = len(rx_buf)

    usb_data = dev.receive(1024 - rx_buf_len)

    if usb_data and len(usb_data) > 0:
        print(f"Received {len(usb_data):d} bytes from USB")
        rx_buf += usb_data
    
    for i, by in enumerate(rx_buf):
        if by == 0x00:
            # Send the complete packet
            handle_received_packet(rx_buf[:i])

            # Shift back the remaining data
            remaining_start = i + 1
            rx_buf = rx_buf[remaining_start:]
            break