# gcode_generation.py
import numpy as np

# parameters
CART_OFFSET = 20 # mm out
SCAN_SET_HEIGHT = 400 # mm up from turntable
A_FEED = 1000 # wrist feed mm/min
MOVE_FEED = 5000 # move instruction feed mm/min
CUT_FEED = 1500 # cut instruction feed mm/min
Z_MAX = 813 # mm
DECS = 3
PARKING_R = 300 # mm
START_POINT = (0, -390, 500, 0) # mm, mm, mm, deg
MIN_R_SLOP = 5
POT_HEIGHT = 0.254
POT_DIAMETER = 0.28
PLANT_KEEPOUT_DIAMETER = POT_DIAMETER * 0.75
TOOLHEAD_LEN = 180

# trimbot dimension constants
D = 477.5
Y_MAX_POS = 330
TY = 158
TZ = 41

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
            x = point[1] * 1000.0
            y = -1 * point[0] * 1000.0
            z = point[2] * 1000.0 + SCAN_SET_HEIGHT
            point[0] = x
            point[1] = y
            point[2] = z
    return paths

def magnitude(x, y):
    dist_from_z = np.sqrt(x**2 + y**2)
    return dist_from_z

def get_trimmer_angle(curr_coord, next_coord):
    r_vec = np.array((curr_coord[0], curr_coord[1], 0))
    r_vec = r_vec / np.linalg.norm(r_vec)
    point_vec = np.subtract(next_coord, curr_coord)
    mag_point_vec = np.linalg.norm(point_vec)
    ang = np.arccos(np.dot(r_vec, point_vec) / mag_point_vec) * (180.0 / np.pi) - 90
    ang = np.clip(ang, 0, 180)

    # override the angle if the point is too close in to allow trimbot
    # to reach the point. This is since the extruder can only reach points
    # near the center if the cutter is flat.
    # From trimbot:
    # MIN_R = D - Y_MAX_POS - TY * np.sin(np.deg2rad(ang)) - TZ * np.cos(np.deg2rad(ang))
    min_r = D - Y_MAX_POS - TY * np.sin(np.deg2rad(ang)) - TZ * np.cos(np.deg2rad(ang))
    if magnitude(curr_coord[0], curr_coord[1]) <= min_r + MIN_R_SLOP:
        ang = 90

    # Finally, we also need to change the cutter angle to be shallower if
    # we are near the pot and the bottom surface of the toolhead assembly will hit the plant.
    if curr_coord[2] < POT_HEIGHT + (TOOLHEAD_LEN*np.cos(np.deg2rad(ang))):
        ang = max(ang, np.rad2deg(np.arccos((curr_coord[2] - POT_HEIGHT) / TOOLHEAD_LEN)))
        
    return ang

def radial_offset(f, coord):
    x, y, z = coord
    r = magnitude(x, y)
    scale = (CART_OFFSET + r) / r
    offset_coord = [scale * x, scale * y, z]
    cart_move(f, offset_coord, MOVE_FEED)

def parking_radius(file, curr_coord):
    theta = np.arctan2(curr_coord[0], -1*curr_coord[1])
    x = PARKING_R * np.sin(theta)
    y = -PARKING_R * np.cos(theta)
    file.write("G1 X"+str(round(x, DECS))+ \
               " Y"+str(round(y, DECS))+ \
               " F"+str(MOVE_FEED)+"\n")
    return x, y
    
def linear_z(file, z):
    file.write("G1 Z"+str(round(z, DECS))+ \
               " F"+str(MOVE_FEED)+"\n")

def rotation_move(file, theta, current_point):
    r = np.linalg.norm(current_point[:2])
    X = r * np.sin(theta)
    Y = -r * np.cos(theta)
    I = -current_point[0]
    J = -current_point[1]
    file.write("G3 X"+str(round(X, DECS))+ \
            " Y"+str(round(Y, DECS))+ \
            " I"+str(round(I, DECS))+ \
            " J"+str(round(J, DECS))+ \
            " F"+str(MOVE_FEED)+"\n")

def generate_gcode(paths, fname="out.gcode"):
    
    # open file
    f = open(fname, "w")

    # header
    f.write("G90 ; absolute positions\n")
    f.write("G21 ; mm system units\n")

    # x: r sin theta
    # y: -r cos theta

    # go to initial parking radius
    x, y = parking_radius(f, START_POINT)

    # go to initial point angle
    theta = np.arctan2(paths[0][0][0], -1*paths[0][0][1])
    rotation_move(f, theta, (x, y))

    for i in range(len(paths)):
        # path definition
        path = paths[i]

        # go to z of first point
        linear_z(f, path[0][2])
        
        # enable trimmer
        set_trimmer(f, True)

        if len(path) > 1:
            angle = get_trimmer_angle(path[0], path[1])
            wrist_move(f, angle)

        for n in range(len(path)):
            
            # move to the coord
            cart_move(f, path[n], CUT_FEED)

            # if not last point, use next point to adjust cutter angle
            if(n < len(path) - 1):
                angle = get_trimmer_angle(path[n], path[n+1])
                wrist_move(f, angle)

        # go to parking position
        x, y = parking_radius(f, path[n])

        # disable trimmer
        set_trimmer(f, False)

        # rotation move
        if (i < len(paths) - 1):
            next_path = paths[i+1]
            theta = np.arctan2(next_path[0][0], -1*next_path[0][1])
            rotation_move(f, theta, (x, y))

    # footer
    f.write("G0 A0 X0 Y0 Z"+str(Z_MAX)+" ; rapid end state\n")
    f.write("M84 ; disable steppers\n")

    # close file
    f.close()