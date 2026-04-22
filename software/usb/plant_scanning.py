import time
from enum import Enum
from gcode_sender import gcode_sender_send_gcode
from camera import save_image
from pi_control import pi_control_is_moving
from packet import packet_send, PACKET_TYPE_DONE_SCANNING

WAIT_TIME_BEFORE_MOVE_CHECK = 2000
SCAN_ANGLE_STEP = 30

class ScanStatus(Enum):
    PLANT_SCANNING_IDLE = 0
    PLANT_SCANNING_SCANNING = 1
    PLANT_SCANNING_ERROR = 2

scan_status = ScanStatus.PLANT_SCANNING_IDLE
scan_waiting_for_move = True
scan_last_wait_time = time.time() * 1000
scan_angle = 0

def start_scan():
    global scan_status
    global scan_last_wait_time

    if scan_status == ScanStatus.PLANT_SCANNING_IDLE:
        # Ensure we are at the start position for scanning
        gcode_sender_send_gcode(f"G0 B{scan_angle:d}")
        gcode_sender_send_gcode(f"G0 Z450")
        gcode_sender_send_gcode(f"G0 A132")
        scan_last_wait_time = time.time() * 1000
        scan_status = ScanStatus.PLANT_SCANNING_SCANNING

def plant_scanning_task(usb_dev):
    global scan_status
    global scan_waiting_for_move
    global scan_last_wait_time
    global scan_angle

    match scan_status:
        case ScanStatus.PLANT_SCANNING_IDLE:
            pass

        case ScanStatus.PLANT_SCANNING_SCANNING:
            if scan_waiting_for_move:
                if (time.time() * 1000) - scan_last_wait_time >= WAIT_TIME_BEFORE_MOVE_CHECK:
                    if not pi_control_is_moving():
                        scan_waiting_for_move = False
            else:
                save_image(scan_angle)
                scan_angle += SCAN_ANGLE_STEP
                
                if scan_angle == 360:
                    scan_angle = 0
                    scan_status = ScanStatus.PLANT_SCANNING_IDLE

                    # Tell the MCU that scanning is done
                    id = PACKET_TYPE_DONE_SCANNING
                    packet_send(usb_dev, bytes(), id)
                    return

                gcode_sender_send_gcode(f"G0 B{scan_angle:d}")
                scan_last_wait_time = time.time() * 1000
                scan_waiting_for_move = True

        case ScanStatus.PLANT_SCANNING_ERROR:
            pass