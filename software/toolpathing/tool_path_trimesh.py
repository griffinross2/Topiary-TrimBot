import trimesh as tm
import pyvista as pv
import pymeshlab as pml
import numpy as np

import pynanoinstantmeshes as pynim

# pyvista point cloud reconstruction
cloud = pv.read("point_cloud.vtk")
surf = cloud.reconstruct_surface()
surf.save("surf.ply")
#surf.plot()

# trimesh manipulation
surf = tm.load("surf.ply")
#mesh = tm.load("cube.obj")
mesh = tm.load("cube-octahedron-compound.stl")

def fit_mesh(mesh, surf):
    # center both meshes
    mesh.apply_translation(-mesh.centroid)
    surf.apply_translation(-surf.centroid)
    # TODO: center mesh to surf centroid

    # scale mesh larger than surface
    mesh_scale = (surf.extents / mesh.extents).min()
    mesh.apply_scale(mesh_scale)

    # collision setup
    cm = tm.collision.CollisionManager()
    cm.add_object('container', surf)

    # scales and rotations
    scales = np.linspace(1,0.01,100)
    rotations = np.linspace(0.0,360.0,100) # degrees

    # fitting operation
    for scale in scales:
        # scale
        s = np.diag([scale, scale, scale, 1])

        for rotation in rotations:
            # copy mesh
            mesh_test = mesh.copy()
        
            # rotation
            ang = np.radians(rotation)
            r_z = tm.transformations.rotation_matrix(ang, [0,0,1])
        
            # apply translation
            trans = r_z @ s
            mesh_test.apply_transform(trans)

            # check collision
            collision = cm.in_collision_single(mesh_test)

        if (not collision):
            break
    
    if (collision):
        raise RuntimeError("No fits")
    else:
        return mesh_test

mesh = fit_mesh(mesh,surf)

# create cut list
cut_width = 0.1 # m - effective width of cutting blade
cut_ang = 90 # degrees - angle of plade wrt cutting plane
cut_depth = 0.005 # m - depth of each cut pass

# find theta division
extents = surf.bounding_box.extents
r_avg = (extents[0] + extents[1]) / 2 # crude
theta_0 = np.arccos(1 - ((cut_width ** 2) / (2 * (r_avg ** 2))))
n_divisions = np.floor((2 * np.pi) / theta_0)
theta = (2 * np.pi) / n_divisions
# TODO: theta changes with passes, r_avg decreases while trimming occurs

# 1) find max depth to cut, find integer # of max cut passes
# 2) find theta division per pass shell
# 3) fit cut path for each shell pass vertical strip
# 4) create toolpath list
# 5) transform coordinate system to trimbot home relative
# 6) create gcode from toolpath list and hooks

# visualize
mesh.visual = tm.visual.ColorVisuals(mesh)
mesh.visual.face_colors = [255, 0, 0, 150]
surf.visual.face_colors = [0, 255, 0, 150]
scene = tm.Scene([mesh,surf])
scene.show()
