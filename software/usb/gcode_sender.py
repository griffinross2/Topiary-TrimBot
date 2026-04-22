from packet import *
from usb import USBDev
from enum import Enum
import time

class GCodeSenderStatus(Enum):
    GCODE_SENDER_STATUS_IDLE = 0
    GCODE_SENDER_STATUS_SENDING = 1
    GCODE_SENDER_STATUS_ERROR = 2

gcode_sender_status = GCodeSenderStatus.GCODE_SENDER_STATUS_IDLE
gcode_sender_waiting_for_ack = False
gcode_sender_gcode_buf = []
gcode_sender_ack_timeout_tick_ms = 0

GCODE_SENDER_ACK_TIMEOUT = 500

def gcode_sender_task(dev: USBDev):
    global gcode_sender_status
    global gcode_sender_waiting_for_ack
    global gcode_sender_gcode_buf
    global gcode_sender_ack_timeout_tick_ms

    match gcode_sender_status:
        case GCodeSenderStatus.GCODE_SENDER_STATUS_IDLE:
            pass

        case GCodeSenderStatus.GCODE_SENDER_STATUS_SENDING:
            if not gcode_sender_waiting_for_ack:
                if gcode_sender_buffer_empty():
                    gcode_sender_status = GCodeSenderStatus.GCODE_SENDER_STATUS_IDLE
                    return

                id = PACKET_TYPE_GCODE
                res = packet_send(dev, bytes(gcode_sender_gcode_buf[0], encoding='utf-8'), id)

                print(f"Sent gcode: {gcode_sender_gcode_buf[0]}")

                if res >= 0:
                    gcode_sender_waiting_for_ack = True
                    gcode_sender_ack_timeout_tick_ms = int(time.time()*1000)

            else:
                if (int(time.time()*1000) - gcode_sender_ack_timeout_tick_ms) >= GCODE_SENDER_ACK_TIMEOUT:
                    # Timeout
                    gcode_sender_waiting_for_ack = False
                    gcode_sender_status = GCodeSenderStatus.GCODE_SENDER_STATUS_ERROR

        case GCodeSenderStatus.GCODE_SENDER_STATUS_ERROR:
            pass

        case _:
            pass

def gcode_sender_send_gcode(gcode):
    global gcode_sender_status
    global gcode_sender_gcode_buf
    
    gcode_sender_gcode_buf.append(gcode)
    gcode_sender_status = GCodeSenderStatus.GCODE_SENDER_STATUS_SENDING

def gcode_sender_buffer_empty():
    return len(gcode_sender_gcode_buf) == 0

def gcode_sender_give_packet(id: PacketID, data: bytes):
    global gcode_sender_status
    global gcode_sender_waiting_for_ack
    global gcode_sender_gcode_buf

    match id.packet_type:
        case PACKET_TYPE_GCODE.packet_type:
            if id.ack:
                gcode_sender_waiting_for_ack = False
                if len(gcode_sender_gcode_buf) > 1:
                    gcode_sender_gcode_buf = gcode_sender_gcode_buf[1:]
                else:
                    gcode_sender_gcode_buf = []
            else:
                gcode_sender_status = GCodeSenderStatus.GCODE_SENDER_STATUS_ERROR