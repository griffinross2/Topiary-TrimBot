import numpy as np
DEPTH_CORRECTION = 1

def disp_to_depth(disparity, image_width, d_stereo=4, fov=128):
    # print(disparity, image_width)
    depth = DEPTH_CORRECTION * (image_width * d_stereo) / (2 * disparity * np.tan(np.radians(fov / 2)))
    return depth