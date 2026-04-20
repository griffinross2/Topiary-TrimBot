from PIL import Image,ImageFilter
import colorsys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colormaps as cm

from create_outline import *
from get_disparity_level import *
from picture_prep import *
from disp_to_depth import *
from get_point_on_outline import *
from project_outline_to_3d import *

def scan_plant(img_left, img_right, angle):

    img_left, w_left, h_left = picture_prep(img_left)
    silhouette_left, outline_left = create_outline(img_left)
    outline_left = np.array(outline_left)
    

    img_right, w_right, h_right = picture_prep(img_right)
    silhouette_right, outline_right = create_outline(img_right)

    disparity = get_disparity_level(silhouette_right, silhouette_left)
    print("disp: ", disparity)
    depth = disp_to_depth(disparity, w_left)
    print("depth: ", depth)

    pts = project_outline_to_3d(outline_left, 4, disparity, depth, angle, w_left, h_left)
    pts = np.array(pts)

    return(pts)
