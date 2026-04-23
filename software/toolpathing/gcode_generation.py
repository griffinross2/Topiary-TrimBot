# gcode_generation.py
import numpy as np

# parameters
CART_OFFSET = 20 # mm out
SCAN_SET_HEIGHT = 450 # mm down from home
A_FEED = 1000 # wrist feed mm/min
MOVE_FEED = 5000 # move instruction feed mm/min
CUT_FEED = 2000 # cut instruction feed mm/min
Z_MAX = 813 # mm
DECS = 3

# trimmer codes
ENABLE_TRIMMER = "M991"
DISABLE_TRIMMER = "M992"

# helper functions
def cart_move(file, coord, feed):
    x,y,z = coord
    x = round(x, DECS)
    y = round(y, DECS)
    z = round(z, DECS)
    file.write("G1 X"+str(x)+" Y"+str(y)+" Z"+str(z)+" F"+str(feed)+"\n")

def wrist_move(file, angle, feed=A_FEED):
    angle = round(angle, DECS)
    file.write("G1 A"+str(angle)+" F"+str(feed)+"\n")

def set_trimmer(file, is_en):
    if (is_en):
        file.write(ENABLE_TRIMMER+"\n")
    else:
        file.write(DISABLE_TRIMMER+"\n")

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

def radial_offset(f, coord):
    x, y, z = coord
    r = magnitude(x, y)
    scale = (CART_OFFSET + r) / r
    offset_coord = [scale * x, scale * y, z]
    cart_move(f, offset_coord, MOVE_FEED)

def generate_gcode(paths, fname="out.gcode"):
    
    # open file
    f = open(fname, "w")

    # header
    f.write("G90 ; absolute positions\n")
    f.write("G21 ; mm system units\n")

    for path in paths:
        
        # move cutter to offset radially from first point
        radial_offset(f, path[0])

        # enable trimmer
        set_trimmer(f, True)

        for n in range(len(path)):
            
            # if not last point, use next point to adjust cutter angle
            if(n < len(path) - 1):
                angle = get_trimmer_angle(path[n], path[n+1])
                wrist_move(f, angle)
            
            # move to next coord
            cart_move(f, path[n], CUT_FEED)

        # move cutter out
        radial_offset(f, path[n])

        # disable trimmer
        set_trimmer(f, False)

    # footer
    f.write("G0 A0 X0 Y0 Z"+str(Z_MAX)+" ; rapid end state\n")
    f.write("M84 ; disable steppers\n")

    # close file
    f.close()
