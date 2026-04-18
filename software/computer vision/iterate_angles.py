import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colormaps as cm
from pyvista import examples

from scan_plant import *
from correction import *

def iterate_angles():
    pt_cloud = []

    correction()

    img_left = Image.open("left_rot_1.jpg")
    img_right = Image.open("right_rot_1.jpg")
    pts = scan_plant(img_left, img_right, 0)
    pt_cloud.extend(pts)

    img_left = Image.open("left_rot_2.jpg")
    img_right = Image.open("right_rot_2.jpg")
    pts = scan_plant(img_left, img_right, 30)
    pt_cloud.extend(pts)

    img_left = Image.open("left_rot_3.jpg")
    img_right = Image.open("right_rot_3.jpg")
    pts = scan_plant(img_left, img_right, 60)
    pt_cloud.extend(pts)

    img_left = Image.open("left_rot_4.jpg")
    img_right = Image.open("right_rot_4.jpg")
    pts = scan_plant(img_left, img_right, 90)
    pt_cloud.extend(pts)


    img_left = Image.open("left_rot_4.jpg")
    img_right = Image.open("right_rot_4.jpg")
    pts = scan_plant(img_left, img_right, 150)
    pt_cloud.extend(pts)


    ## PREPARE DATA FOR PLOTS
    pt_cloud = pv.PolyData(pt_cloud)
    #pt_cloud.save('point_cloud.vtk')
    recon = pt_cloud.reconstruct_surface()

    global surf
    surf = pv.wrap(pt_cloud.delaunay_3d())
    surf_bounds = surf.bounds

    global model
    model = examples.download_action_figure()
    model = model.rotate_x(90)
    model_bounds = model.bounds

    z_min = max(model_bounds[4],surf_bounds[4]) + 1
    z_max = min(model_bounds[5],surf_bounds[5]) - 1
    ##

    global plotter
    plotter = pv.Plotter(shape=(2,2))
    plotter.show_axes()

    plotter.subplot(0,0)
    plotter.show_axes()
    plotter.camera.position = (400, 0, 0)
    plotter.add_mesh(pt_cloud)
    plotter.add_mesh(pv.Sphere(radius=.5))

    plotter.subplot(0,1)
    plotter.show_axes()
    plotter.camera.position = (400, 0, 0)
    plotter.add_mesh(recon, color=True, show_edges=True)

    plotter.subplot(1,0)
    plotter.show_axes()
    plotter.camera.position = (400, 0, 0)
    plotter.add_mesh(surf, show_edges=True, color='green')
    plotter.add_mesh(model, color='red')

    plotter.subplot(1,1)
    plotter.show_axes()
    plotter.camera.position = (400, 0, 0)
    plotter.add_mesh(surf, show_edges=True, opacity=.1, color='green')
    plotter.add_mesh(model, color='red', opacity=.1)
    plotter.add_slider_widget(create_slice, [z_min, z_max], title='Level',interaction_event='always')

    plotter.show()

    return

def create_slice(level):
    slice_surf = surf.slice(normal=[0,0,1], origin=[0,0,level])
    plotter.add_mesh(slice_surf, name='slice_surf', color='green')
    slice_model = model.slice(normal=[0,0,1], origin=[0,0,level])
    plotter.add_mesh(slice_model,color='red', name='slice_model')

iterate_angles()