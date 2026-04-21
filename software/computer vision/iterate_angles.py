import numpy as np
import matplotlib.pyplot as plt
import glob
from matplotlib import colormaps as cm
from pyvista import examples
import trimesh as tm
from PIL import Image, ImageOps

from scan_plant import *
from correction import *

ANGLE_STEP = 30

def iterate_angles():
    pt_cloud = []

    # correction() # MUST RUN AFTER CHANGING CALIBRATION IMAGES

    angle = 0
    path = '/Users/duke1/OneDrive/Documents/GitHub/Topiary-TrimBot/software/computer vision/plants/'
    
    global points_2d # stores 2D points separated by image angle
    points_2d = []

    # Scan each pair of images
    for angle in range(0, 360, ANGLE_STEP):
        left_filename = path + f"left_{angle:d}.jpg"
        right_filename = path + f"right_{angle:d}.jpg"
        img_left = Image.open(left_filename)
        img_left = ImageOps.exif_transpose(img_left)
        img_right = Image.open(right_filename)
        img_right = ImageOps.exif_transpose(img_right)

        pts = scan_plant(img_left, img_right, angle)

        pt_cloud.extend(pts)
        points_2d.append(pts)


    


    #---- PREPARE PLOT DATA ----#
    #< plotting functionality only below this line >#
        
    global recon
    cloud = tm.PointCloud(pt_cloud)
    mesh = cloud.convex_hull
    mesh.export("plant.ply")
    recon = pv.read("plant.ply")

    # print("Plant bounds: ", recon.bounds)
    # print("Plant length, width, height: ", recon.extents)

    pt_cloud = pv.PolyData(pt_cloud)

    # Model is used to test slicer
    global model
    model = pv.Cube(x_length=10,y_length=15,z_length=20)
    model_bounds = model.bounds

    recon_bounds = recon.bounds

    z_min = max(model_bounds[4],recon_bounds[4]) + 1
    z_max = min(model_bounds[5],recon_bounds[5]) - 1





    #---- BEGIN PLOTTING ----#

    global plotter
    plotter = pv.Plotter(shape=(1,2))
    plotter.show_axes()

    # Plot 2D outlines with slider to show individual outlines
    plotter.subplot(0,0)
    # Turntable and pot represented as cylinders for reference
    table = pv.Cylinder(center = [0,0,-16], direction = [0,0,1], radius=6,height=.5)
    pot = pv.Cylinder(center = [0,0,4.5+-16], direction = [0,0,1], radius=5,height=9)
    plotter.show_axes()
    plotter.camera.position = (200, 0, 0)
    plotter.add_mesh(pt_cloud, opacity = .1)
    plotter.add_mesh(table,color='brown')
    plotter.add_mesh(pot,color='gray',opacity=.5)
    plotter.add_slider_widget(show_2D, [0, 360 / ANGLE_STEP - 1], title='angle', interaction_event='always', value=6)
    plotter.add_mesh(pv.Sphere(radius=.5, center=[6,0,-16]))

    # Plot Trimesh reconstruction
    plotter.subplot(0,1)
    plotter.show_axes()
    plotter.camera.position = (200, 0, 0)
    plotter.add_mesh(recon, show_edges=True, color='green')
    plotter.add_mesh(table,color='brown')
    plotter.add_mesh(pot,color='gray',opacity=.5)
    plotter.add_mesh(pv.Sphere(radius=.5, center=[6,0,-16]))

    # # Plot Trimesh reconstruction and model
    # plotter.subplot(1,0)
    # plotter.show_axes()
    # plotter.camera.position = (200, 0, 0)
    # plotter.add_mesh(recon, show_edges=True, color='green')
    # plotter.add_mesh(model, color='red')

    # # Plot slicer with slider to change height
    # plotter.subplot(1,1)
    # plotter.show_axes()
    # plotter.camera.position = (200, 0, 0)
    # plotter.add_mesh(recon, show_edges=True, opacity=.1, color='green')
    # plotter.add_mesh(model, color='red', opacity=.1)
    # plotter.add_slider_widget(create_slice, [z_min, z_max], title='Level',interaction_event='always')

    plotter.show()

    return





#---- SLIDER WIDGET FUNCTIONS ----#

def create_slice(level): # slicer widget
    plotter.subplot(1,1)
    slice_recon = recon.slice(normal=[0,0,1], origin=[0,0,level])
    plotter.add_mesh(slice_recon, name='slice_recon', color='green')
    slice_model = model.slice(normal=[0,0,1], origin=[0,0,level])
    plotter.add_mesh(slice_model,color='red', name='slice_model')

def show_2D(value): # change 2D point angle widget
    plotter.subplot(0,0)
    i = int(value)
    face = points_2d[i]
    face = pv.PolyData(face)
    plotter.add_mesh(face, name='face', color='black')

iterate_angles()