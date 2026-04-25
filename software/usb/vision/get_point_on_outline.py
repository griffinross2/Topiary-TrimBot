from PIL import Image,ImageFilter
import colorsys
import numpy as np
import matplotlib.pyplot as plt

def get_point_on_outline(image_left: Image.Image, angle):
    # Binary search for the border of black and white
    width, height = image_left.size
    r_interval = [0, width - 1]
    while r_interval[1] - r_interval[0] > 1:
        r_mid = (r_interval[0] + r_interval[1]) // 2
        x_mid = int(width // 2 + r_mid * np.cos(np.radians(angle)))
        y_mid = height - int(height // 2 + r_mid * np.sin(np.radians(angle)))

        # if we are outside of the image, take the inner interval and continue
        if x_mid < 0 or x_mid >= width or y_mid < 0 or y_mid >= height:
            r_interval[1] = r_mid
            continue

        pixel = image_left.getpixel((x_mid, y_mid))
        if pixel == (0,0,0):  # black
            r_interval[0] = r_mid
        else:  # white
            r_interval[1] = r_mid

    r_final = r_interval[1]
    x_final = width // 2 + r_final * np.cos(np.radians(angle))
    y_final = height - (height // 2 + r_final * np.sin(np.radians(angle)))
    return (x_final, y_final)