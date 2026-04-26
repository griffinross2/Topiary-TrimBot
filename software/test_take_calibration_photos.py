from usb import USBDev
import signal
import packet
import camera
import packet
import serial
import time
from packet_engine import packet_engine_task
from pi_control import pi_control_is_moving
from gcode_sender import gcode_sender_task, gcode_sender_send_gcode

should_quit = False
usb_dev = USBDev()
photos_index = 0
status = 0

def sigint_handler(signum, frame):
    print("SIGINT received, exiting...")
    usb_dev.disconnect()
    camera.deinit_cameras()
    quit(0)

def take_photos_task():
    global photos_index
    global status

    if status == 0:
        gcode_sender_send_gcode("G0 Z500 Y-250 A135")
        time.sleep(2)
        status = 1

    if status == 1 and not pi_control_is_moving():
        status = 2

    if status == 2:
        camera.save_image(0, f"calibration_images/left/{photos_index}.jpg", f"calibration_images/right/{photos_index}.jpg")
        gcode_sender_send_gcode("G0 A90")
        gcode_sender_send_gcode("G0 A135")
        time.sleep(2)
        status = 3

    if status == 3 and not pi_control_is_moving():
        photos_index += 1
        if photos_index >= 20:
            usb_dev.disconnect()
            camera.deinit_cameras()
            quit(0)
        status = 2


def main():
    global usb_dev
    global should_quit
    
    signal.signal(signal.SIGINT, sigint_handler)
    
    try:
        usb_dev.connect()
    except serial.SerialException as e:
        print(f"Failed to connect: {e}")
        raise e
    
    camera.init_cameras()

    print("Init Complete")

    while True:
        packet_engine_task(usb_dev)
        gcode_sender_task(usb_dev)
        take_photos_task()

if __name__ == "__main__":
    while True:
        try:
            main()
        except Exception as e:
            print("Error, probably disconnection: " + str(e))
            # Make sure to release the cameras
            try:
                camera.deinit_cameras()
            except Exception:
                pass

