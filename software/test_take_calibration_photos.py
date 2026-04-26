from usb import USBDev
import signal
import packet
import camera
import packet
import serial
import time
from packet_engine import packet_engine_task
from pi_control import pi_control_is_moving

should_quit = False
usb_dev = USBDev()

def sigint_handler(signum, frame):
    global should_quit
    should_quit = True

def quit_main():
    global usb_dev
    
    print("SIGINT received, exiting...")
    usb_dev.disconnect()
    camera.deinit_cameras()
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
    
    camera.init_cameras()

    print("Init Complete")

    time.sleep(1*60)
    id = packet.PACKET_TYPE_GCODE
    packet.packet_send(usb_dev, bytes("G1 Y-220 Z500 A135", 'utf-8'), id)
    time.sleep(5)

    i = 0
    while(True):
        time.sleep(1)
        camera.save_image(0, f"calibration_images/left/{i}.jpg", f"calibration_images/right/{i}.jpg")
        packet.packet_send(usb_dev, id, "A90")
        packet.packet_send(usb_dev, id, "A135")
        time.sleep(1)
        packet_engine_task(usb_dev)
        while pi_control_is_moving():
            time.sleep(0.1)
            packet_engine_task(usb_dev)
            
        i += 1

        if (should_quit or i >= 20):
            quit_main()

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

