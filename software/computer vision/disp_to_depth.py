import numpy as np

def disp_to_depth(disparity, image_width=1000, d_stereo=5, fov=150):
    depth = (image_width * d_stereo) / (2 * disparity * np.tan(np.radians(fov / 2)))
    return depth