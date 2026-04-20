import numpy as np

def disp_to_depth(disparity, image_width, d_stereo=4, fov=160):
    print(disparity, image_width)
    depth = (image_width * d_stereo) / (2 * disparity * np.tan(np.radians(fov / 2)))
    return depth