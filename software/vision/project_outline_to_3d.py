import numpy as np

# X_SCALE = (21/22)
# Y_SCALE = (2/3)
X_SCALE = 1
Y_SCALE = 1

def project_outline_to_3d(outline_points, camera_orig_dist, disp, depth, angle, org_width, org_height, width, height, vert_fov=85, hor_fov=128):
    
    x_scale = X_SCALE * 2 * depth * np.tan(np.radians(hor_fov / 2)) / org_width
    y_scale = Y_SCALE * 2 * depth * np.tan(np.radians(vert_fov / 2)) / org_height
    r = camera_orig_dist - depth
    theta = np.deg2rad(angle)

    p0 = np.array([r*np.cos(theta), r*np.sin(theta), 0.0])
    x_axis = np.array([-np.sin(theta), np.cos(theta), 0.0])
    y_axis = np.array([0.0, 0.0, 1.0])

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
        pt = p0 + x_inch*x_axis + y_inch*y_axis
        
        ## CONDITION TO DECREASE POINT SAMPLING:
        # if(i % 10 == 0): 
        pts_3d.append(pt)
        i = i + 1

    return pts_3d