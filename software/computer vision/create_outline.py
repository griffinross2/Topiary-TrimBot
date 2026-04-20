from PIL import Image
import colorsys
import numpy as np
import matplotlib.pyplot as plt

from matplotlib import colormaps as cm
from matplotlib.path import Path

def create_outline(img):
    # values to tweak: lowest & highest hue, 
    #   lowest sat, contour "levels" arg,

    cmap = cm.get_cmap('plasma') 

    map = np.zeros((img.width, img.height))

    reference_hue = 100 #green = 120, yellow = 60

    lowest_hue = -50
    lowest_hue = np.deg2rad(lowest_hue)
    highest_hue = 50
    highest_hue = np.deg2rad(highest_hue)

    lowest_sat = .2
    lowest_val = .1      # value filtering included, but not used

    for x in range(img.width):
        for y in range(img.height):
            r, g, b = img.getpixel((x, y))
            h, s, v = colorsys.rgb_to_hsv(r,g,b)
            h = h * 360 - reference_hue
            h = np.deg2rad(h)

            if(lowest_hue < h < highest_hue):
                excess_green = np.cos(h)
            else:
                excess_green = 0
            if(s < lowest_sat or v < lowest_val):
                excess_green = 0
            

            map[x, y] = excess_green

    # Normalize and reorient
    map = map/np.max(map)
    map = map.T

    # Filter low values
    threshold = 0.2
    map[map < threshold] = 0

    # Show map
    # map_colored = (cmap(map)[:, :, :3]*255).astype(np.uint8)
    # map_img = Image.fromarray(map_colored, 'RGB')
    # map_img.show()

    # create contour "outline"
    contour = plt.contour(map, levels=[.2], origin='image') #best: levels=[.2]
    paths = contour.get_paths()
    path = paths[0]

    points = []
    ops = []
    for vertices, code in path.iter_segments():
        points.append(vertices)
        ops.append(code)

    points = np.array(points)
    ops = np.array(ops)

    length = 0
    max_length = 0
    start_index = 0
    i = 0

    for vertices, code in path.iter_segments():
        length = length + 1

        if code == 1:
            index = i

        if code == 79 or code == 1: # CLOSEPOLY = np.uint8(79)
            length = 0
        if length > max_length:
            max_length = length
            start_index = index
            
        i = i + 1

    outline_verts = points[start_index:start_index + max_length]
    outline_codes = ops[start_index:start_index + max_length]
    outline_path = Path(outline_verts,outline_codes)

    img = img.transpose(Image.FLIP_LEFT_RIGHT)
    img = img.transpose(Image.ROTATE_180)

    for x in range(img.width):
        for y in range(img.height):
            if outline_path.contains_point([x,y]):
                img.putpixel((x, y), (0,0,0,225))
            else:
                img.putpixel((x, y), (225,225,225,225))
    
    img = img.transpose(Image.FLIP_LEFT_RIGHT)
    img = img.transpose(Image.ROTATE_180)

    return(img, outline_verts)