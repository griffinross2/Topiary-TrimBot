import numpy as np

# X_SCALE = (21/22)
# Y_SCALE = (2/3)
X_SCALE = 1
Y_SCALE = 1

def project_outline_to_3d(outline_points, camera_orig_dist, disp, depth, angle, width, height, vert_fov=85, hor_fov=128):
    
    x_scale = X_SCALE * 2 * depth * np.tan(np.radians(hor_fov / 2)) / width
    y_scale = Y_SCALE * 2 * depth * np.tan(np.radians(vert_fov / 2)) / height
    r = camera_orig_dist - depth

    # pick basis vectors
    n = np.array([-1 * np.cos(np.radians(angle)),-1 * np.sin(np.radians(angle)), 0])
    u1 = (0, 0, 1)
    u2 = np.cross(n, u1)
    R = np.array([u2, u1, n]).T

    pts_3d = []

    i = 0
    for (x_2d, y_2d) in outline_points:
        # Get the offsets from the center
        x_offset = x_2d - width / 2 - disp / 2
        y_offset = y_2d - height / 2

        # Scale to inches
        x_inch = x_offset * x_scale
        y_inch = y_offset * y_scale

        # Transform this cross section by placing it on its plane in 3D
        pt = np.matmul(R, np.array([x_inch, y_inch, 0])) + (r * n)
        
        ## CONDITION TO DECREASE POINT SAMPLING:
        # if(i % 10 == 0): 
        pts_3d.append(pt)
        i = i + 1

    return pts_3d