import pyvista as pv
import numpy as np
import io
from PIL import Image
import os
import matplotlib.pyplot as plt
from matplotlib import colormaps as cm

def get_disparity_level(image_left: Image.Image, image_right: Image.Image):
    # just shift the left to the right up to a limit until a maximum correlation is found
    max_shift = 100
    best_shift = 0
    best_correlation = -1.0

    left_array = np.array(image_left.convert('1', dither=Image.NONE))
    right_array = np.array(image_right.convert('1', dither=Image.NONE))
    
    for shift in range(max_shift):
        correlation = np.sum(np.multiply(np.logical_not(left_array), np.logical_not(right_array)))
        left_array = np.roll(left_array, 1, axis=1)

        #both = np.logical_not(np.multiply(np.logical_not(left_array), np.logical_not(right_array)))
        #array_img = Image.fromarray(both)
        #array_img.show()

        if correlation > best_correlation:
            best_correlation = correlation
            best_shift = shift

    return best_shift