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

def scan_plant():

    #calibration call here?

    img_left = Image.open("left_rot_1.jpg")
    img_left, w_left, h_left = picture_prep(img_left)
    silhouette_left, outline_left = create_outline(img_left) # china

    img_right = Image.open("right_rot_1.jpg")
    img_right, w_right, h_right = picture_prep(img_right)
    silhouette_right, outline_right = create_outline(img_right) # tree

    disparity = get_disparity_level(silhouette_right, silhouette_left)

    depth = disp_to_depth(disparity, w_left)
    print(depth)



    
    
    #array_img = Image.fromarray(outline_points)
    #array_img.show()

    # # to move to parent function:
    pts = project_outline_to_3d(outline_left, 18.0, depth, 0)
    points_3d = []
    points_3d.extend(pts)

    plotter = pv.Plotter(shape=(1,1))
    plotter.camera.position = (100, 0, 0)

    pv.plot(points_3d)

    #point_cloud = pv.PolyData(np.array(points_3d))
    #point_cloud.save('point_cloud.vtk')
    #plotter.add_points


    #reconstructed_mesh = point_cloud.reconstruct_surface()
    #plotter.add_mesh(reconstructed_mesh, color='red', show_edges=False)
    #plotter.show()


scan_plant()