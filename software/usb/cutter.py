from gcode_sender import gcode_sender_send_gcode

def start_cutting():
    with open("gcode/out.gcode", "r") as f:
        for line in f.readlines():
            gcode_sender_send_gcode(line)