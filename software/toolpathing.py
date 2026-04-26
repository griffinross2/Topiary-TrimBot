from toolpath.gcode import do_toolpath
from file_receiver import file_receiver_get_model_file_name
from packet import *
from usb import USBDev

done_toolpathing = False

def toolpathing_task(usb_dev: USBDev):
    global done_toolpathing
    if done_toolpathing:
        # Send that we are done scanning
        id = PACKET_TYPE_DONE_TOOLPATHING
        packet_send(usb_dev, bytes(), id)
        done_toolpathing = False

def start_toolpathing():
    global done_toolpathing

    do_toolpath(file_receiver_get_model_file_name())

    done_toolpathing = True