# gcode_generation.py
import numpy as np
import math

CART_OFFSET = 10
SCAN_SET_HEIGHT = 400 # mm down from home
PARKING_Y_OFFSET = 20 # mm away from plan in y

B_FEED = 100 # wrist feed
MOVE_FEED = 100 # 
CUT_FEED = 100 # 

# helper functions
def cart_move(file, coord, feed=MOVE_FEED):
    x,y,z = coord
    file.write("G1 X"+str(x)+" Y"+str(y)+" Z"+str(z)+" F"+str(feed)+"\n")

def wrist_move(file, angle, feed=B_FEED):
    file.write("G1 B"+str(angle)+" F"+str(feed)+"\n")

def transform_data(paths): # transform toolpath to trimbot coords
    for path in paths:
        for point in path:
            point[0] = 1000.0 * point[0]
            point[1] = 1000.0 * point[1]
            point[2] = 1000.0 * point[2] - SCAN_SET_HEIGHT
    return paths

def get_trimmer_angle(curr_coord, next_coord):
    y_vec = [0, 1, 0]
    point_vec = np.subtract(next_coord, curr_coord)
    mag_point_vec = np.linalg.norm(point_vec)
    ang = np.arccos(np.dot(y_vec, point_vec) / mag_point_vec) * (180.0 / np.pi)
    return ang

def magnitude(x, y):
    dist_from_z = np.sqrt(x**2 + y**2)
    return dist_from_z

def generate_gcode(paths):
    # inputs
    fname = "out.gcode"

    # parameters
    

    # open file
    f = open(fname, "w")

    # header
    f.write("G90 ; absolute positions\n")
    f.write("G21 ; mm system units\n")
    # f.write("G28 ; home\n")

    i = 0

    for path in paths:
        
        # move cutter to 10 mm radially from first point
        x_0, y_0, z_0 = path[0]
        r = magnitude(x_0, y_0)
        scale = (CART_OFFSET + r) / r
        start = [scale * x_0, scale * y_0, z_0]

        cart_move(f, start) 

        for n in len(path):
            
            # if not last point, use next point to adjust cutter angle
            if(n < len(path)):
                angle = get_trimmer_angle(path[n], path[n+1])
                wrist_move(f, angle)
            
            cart_move(f, path[n])

        cart_move () # back out and rotate





    # for path in paths:
    #     for point in points:
    #         cart_offset = get_cart_offset()
    #         cart_move(commands, cart_offset, MOVE_FEED) # move to point distance away
    #         angle = # TODO: get angle for current point
    #         wrist_move(commands, angle, MOVE_FEED) # angle wrist
    #         # TODO: enable trimmer
    #         cart_move(commands, point, CUT_FEED) # move to coord
    #         # TODO: disable trimmer
    #         cart_move(commands)  
    #

    # footer
    # TODO: turn off trimmer
    f.write("G0 B0 X0 Y0 Z1000 ; rapid end state\n") # TODO: Z1000?
    f.write("M84 ; disable steppers\n")

    # close file
    f.close()
