import numpy as np
DEPTH_CORRECTION = 1

def disp_to_depth(disparity, image_width, d_stereo=4, hfov=128):
    # print(disparity, image_width)
    depth = DEPTH_CORRECTION * (image_width * d_stereo) / (2 * disparity * np.tan(np.radians(hfov / 2)))
    return depth

def depth_to_disp(depth, image_width, d_stereo=4, hfov=128):
    disparity = DEPTH_CORRECTION * (image_width * d_stereo) / (2 * depth * np.tan(np.radians(hfov / 2)))
    return disparity