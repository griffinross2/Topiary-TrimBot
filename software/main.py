from usb import USBDev
import signal
from packet_engine import packet_engine_task
from file_receiver import file_receiver_task
from gcode_sender import gcode_sender_task
from camera import init_cameras, deinit_cameras
from plant_scanning import plant_scanning_task
from slicer_view import slicer_view_task
from toolpathing import toolpathing_task
import packet
import serial
import time

should_quit = False
usb_dev = USBDev()

def sigint_handler(signum, frame):
    global should_quit
    should_quit = True

def quit_main():
    global usb_dev
    
    print("SIGINT received, exiting...")
    usb_dev.disconnect()
    deinit_cameras()
    quit(0)

def main():
    global usb_dev
    global should_quit
    
    signal.signal(signal.SIGINT, sigint_handler)
    
    try:
        usb_dev.connect()
    except serial.SerialException as e:
        print(f"Failed to connect: {e}")
        raise e

    init_cameras()
    
    print("Init Complete")

    while(True):
        packet_engine_task(usb_dev)
        file_receiver_task(usb_dev)
        gcode_sender_task(usb_dev)
        plant_scanning_task(usb_dev)
        slicer_view_task(usb_dev)
        toolpathing_task(usb_dev)

        if (should_quit):
            quit_main()

if __name__ == "__main__":
    while True:
        try:
            main()
        except Exception as e:
            print("Error, probably disconnection: " + str(e))
            # Make sure to release the cameras
            try:
                deinit_cameras()
            except Exception:
                pass

