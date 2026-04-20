import numpy as np
import matplotlib.pyplot as plt
import glob
from matplotlib import colormaps as cm
from pyvista import examples
import trimesh as tm

from scan_plant import *
from correction import *

ANGLE_STEP = 20

def iterate_angles():
    pt_cloud = []

    #correction()

    angle = 0
    path_left = '/Users/duke1/OneDrive/Documents/GitHub/Topiary-TrimBot/software/computer vision/plants_left/'
    path_right = '/Users/duke1/OneDrive/Documents/GitHub/Topiary-TrimBot/software/computer vision/plants_right/'
    global points_2d
    points_2d = []

    for angle in range(0, 360, ANGLE_STEP):
        left_filename = path_left + f"left_middle_{angle:d}.jpg"
        right_filename = path_right + f"right_middle_{angle:d}.jpg"
    
        img_left = Image.open(left_filename)
        img_right = Image.open(right_filename)
        pts = scan_plant(img_left, img_right, angle)
        pt_cloud.extend(pts)
        points_2d.append(pts)

    cloud = tm.PointCloud(pt_cloud)
    mesh = cloud.convex_hull
    #print(mesh.bounds)
    print(mesh.extents)
    mesh.export("plant.ply")
    recon = pv.read("plant.ply")

    ## PREPARE DATA FOR PLOTS
    pt_cloud = pv.PolyData(pt_cloud)
    #pt_cloud.save('point_cloud.vtk')
    # recon = pt_cloud.reconstruct_surface()

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
    table = pv.Cylinder(center = [0,0,-14], direction = [0,0,1], radius=6,height=.5)
    plotter.show_axes()
    plotter.camera.position = (400, 0, 0)
    plotter.add_mesh(pt_cloud, opacity = .1)
    plotter.add_mesh(table,color='brown')
    plotter.add_slider_widget(show_2D, [0, 17], title='angle',interaction_event='always')
    plotter.add_mesh(pv.Sphere(radius=.5, center=[6,0,-14]))

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
    plotter.subplot(1,1)
    slice_surf = surf.slice(normal=[0,0,1], origin=[0,0,level])
    plotter.add_mesh(slice_surf, name='slice_surf', color='green')
    slice_model = model.slice(normal=[0,0,1], origin=[0,0,level])
    plotter.add_mesh(slice_model,color='red', name='slice_model')

def show_2D(value):
    plotter.subplot(0,0)
    i = int(value)
    face = points_2d[i]
    face = pv.PolyData(face)
    plotter.add_mesh(face, name='face', color='black')

iterate_angles()