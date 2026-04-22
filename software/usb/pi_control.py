from packet import *
from usb import USBDev

currently_moving = False

def pi_control_give_packet(id: PacketID, data: bytes):
    from plant_scanning import start_scan
    global currently_moving
    
    match id.packet_type:
        case PACKET_TYPE_STATUS.packet_type:
            if len(data) == 1:
                currently_moving = int(data[0]) & 0x1 == 1
        
        case PACKET_TYPE_START_SCANNING.packet_type:
            start_scan()

        case _:
            pass

def pi_control_is_moving():
    global currently_moving

    return currently_moving