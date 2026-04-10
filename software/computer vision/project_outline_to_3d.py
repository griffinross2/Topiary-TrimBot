import numpy as np

def project_outline_to_3d(outline_points, camera_dist, depth, angle, fov=120):
    width, height = 1000, 800
    x_scale = 2 * depth * np.tan(np.radians(120 / 2)) / width
    y_scale = 2 * depth * np.tan(np.radians(120 / 2)) / width
    r = camera_dist - depth

    # pick basis vectors
    n = np.array([np.cos(np.radians(angle)), np.sin(np.radians(angle)), 0])
    u1 = (0, 0, 1)
    u2 = np.cross(n, u1)
    R = np.array([u2, u1, n]).T

    points_3d = []
    for (x_2d, y_2d) in outline_points:
        # First get the offsets from the center
        x_offset = x_2d - width / 2 + 53
        y_offset = y_2d - height / 2

        # Scale to inches
        x_inch = x_offset * x_scale
        y_inch = y_offset * y_scale

        # Transform this cross section by placing it on its plane in 3D
        point_3d = np.matmul(R, np.array([x_inch, y_inch, 0])) + (r * n)

        points_3d.append(point_3d)

    return points_3d