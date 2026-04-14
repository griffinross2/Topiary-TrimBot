import numpy as np

def project_outline_to_3d(outline_points, camera_dist, disp, depth, angle, width, height, fov=150):
    
    # x_scale = 2 * depth * np.tan(np.radians(fov / 2)) / width
    # y_scale = 2 * depth * np.tan(np.radians(fov / 2)) / height
    r = camera_dist - depth

    # pick basis vectors
    n = np.array([np.cos(np.radians(angle)), np.sin(np.radians(angle)), 1])

    pts_centered = []
    pts_3d = []

    i = 0
    for (x_2d, y_2d) in outline_points:
        # First get the offsets from the center
        x_offset = x_2d - width / 2 - disp/2
        y_offset = y_2d - height / 2
        if(i % 10 == 0):
            pts_centered.append([x_offset,y_offset])
        i = i + 1

    for (x_2d, y_2d) in pts_centered:
        [x,y,z] = [n[0]*x_2d, n[1]*x_2d, y_2d]
        pts_3d.append([x,y,z])

    return pts_3d