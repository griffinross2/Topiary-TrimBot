from usb import USBDev
import signal
from packet_engine import packet_engine_task
from file_receiver import file_receiver_task
from gcode_sender import gcode_sender_task, gcode_sender_send_gcode
import packet
import serial
import time

should_quit = False
usb_dev = USBDev()
gcode_file = None

def sigint_handler(signum, frame):
    global should_quit
    should_quit = True

def quit_main():
    global usb_dev
    global gcode_file
    
    gcode_file.close()
    print("SIGINT received, exiting...")
    usb_dev.disconnect()
    quit(0)

def main():
    global usb_dev
    global should_quit
    global gcode_file
    
    signal.signal(signal.SIGINT, sigint_handler)
    
    try:
        usb_dev.connect()
    except serial.SerialException as e:
        print(f"Failed to connect: {e}")
        quit(0)

    print("Init Complete")

    gcode_file = open("out.gcode")
    reading = True
    line_count = 0

    while(True):
        packet_engine_task(usb_dev)
        file_receiver_task(usb_dev)
        gcode_sender_task(usb_dev)

        if reading:
            line = gcode_file.readline()
            line_count += 1
            if (line_count % 10) == 0:
                print(f"On line {line_count}")
            if line != "":
                gcode_sender_send_gcode(line)
            else:
                reading = False

        if (should_quit):
            quit_main()

if __name__ == "__main__":
    main()