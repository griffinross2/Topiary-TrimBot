import time
from enum import Enum
from gcode_sender import gcode_sender_send_gcode
from camera import save_image
from pi_control import pi_control_is_moving
from packet import packet_send, PACKET_TYPE_DONE_SCANNING
from vision.iterate_angles import iterate_angles
from mesh_fitter import get_final_meshes, calc_final_meshes
from slicer_view import slicer_view_give_meshes
import numpy as np

WAIT_TIME_BEFORE_MOVE_CHECK = 2000
SCAN_ANGLE_STEP = 30
CAMERA_HEIGHT_OFFSET = 95
SCANNING_RADIUS = 390
FINAL_POS_HEIGHT = 500
FINAL_POS_RADIUS = 390

class ScanStatus(Enum):
    PLANT_SCANNING_IDLE = 0
    PLANT_SCANNING_SCANNING = 1
    PLANT_SCANNING_ERROR = 2

scan_status = ScanStatus.PLANT_SCANNING_IDLE
scan_waiting_for_move = True
scan_last_wait_time = time.time() * 1000
scan_angle = 0

def start_scan(height=400):
    global scan_status
    global scan_last_wait_time

    if scan_status == ScanStatus.PLANT_SCANNING_IDLE:
        # Ensure we are at the start position for scanning
        gcode_z = height - CAMERA_HEIGHT_OFFSET
        gcode_sender_send_gcode(f"G0 Z{gcode_z:d} A135 Y-{SCANNING_RADIUS:d}")
        scan_last_wait_time = time.time() * 1000
        scan_status = ScanStatus.PLANT_SCANNING_SCANNING

def plant_scanning_init():
    global scan_status
    global scan_waiting_for_move
    global scan_last_wait_time
    global scan_angle

    scan_status = ScanStatus.PLANT_SCANNING_IDLE
    scan_waiting_for_move = True
    scan_last_wait_time = time.time() * 1000
    scan_angle = 0

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
                pre_move_x = SCANNING_RADIUS * np.sin(np.deg2rad(scan_angle))
                pre_move_y = -SCANNING_RADIUS * np.cos(np.deg2rad(scan_angle))

                scan_angle += SCAN_ANGLE_STEP
                
                if scan_angle == 360:
                    scan_angle = 0
                    scan_status = ScanStatus.PLANT_SCANNING_IDLE
                    # Make sure to go to the idle position
                    post_move_x = SCANNING_RADIUS * np.sin(np.deg2rad(scan_angle))
                    post_move_y = -SCANNING_RADIUS * np.cos(np.deg2rad(scan_angle))
                    gcode_sender_send_gcode(f"G3 X{post_move_x:.2f} Y{post_move_y:.2f} I{-pre_move_x:.2f} J{-pre_move_y:.2f}")
                    gcode_sender_send_gcode(f"G4 P100")
                    gcode_sender_send_gcode(f"G1 X0 Y-300 Z{FINAL_POS_HEIGHT}")
                    gcode_sender_send_gcode(f"G4 P100")
                    gcode_sender_send_gcode(f"G1 A0")
                    gcode_sender_send_gcode(f"G4 P100")
                    gcode_sender_send_gcode(f"G1 Y-{FINAL_POS_RADIUS}")
                    gcode_sender_send_gcode(f"G4 P100")

                    # Create the plant mesh
                    iterate_angles(SCAN_ANGLE_STEP)

                    # Fit the models
                    calc_final_meshes()
                    plant_mesh, model_mesh = get_final_meshes()
                    slicer_view_give_meshes(plant_mesh, model_mesh)

                    # Clear shit that piled up
                    usb_dev.clear()

                    # We are ready to go to the slicer view
                    id = PACKET_TYPE_DONE_SCANNING
                    packet_send(usb_dev, bytes(), id)

                    return

                post_move_x = SCANNING_RADIUS * np.sin(np.deg2rad(scan_angle))
                post_move_y = -SCANNING_RADIUS * np.cos(np.deg2rad(scan_angle))
                gcode_sender_send_gcode(f"G3 X{post_move_x:.2f} Y{post_move_y:.2f} I{-pre_move_x:.2f} J{-pre_move_y:.2f}")
                scan_last_wait_time = time.time() * 1000
                scan_waiting_for_move = True

case ScanStatus.PLANT_SCANNING_ERROR:
            pass
