import trimesh as tm
import pyvista as pv
import pymeshlab as pml
import numpy as np

# import pynanoinstantmeshes as pynim

# pyvista point cloud reconstruction
cloud = pv.read("point_cloud.vtk")
surf = cloud.reconstruct_surface()
surf.save("surf.ply")
#surf.plot()

# trimesh manipulation
surf = tm.load("surf.ply")
mesh = tm.load("cube.obj")
# mesh = tm.load("cube-octahedron-compound.stl")

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
cut_ang = 90 # degrees - angle of blade wrt cutting plane
cut_depth = 0.01 # m - depth of each cut pass
patch_height = 0.05 # m - height of patches used to cover the surface
patch_width = cut_width # m - width of patches used to cover the surface
patch_subdivisions = 10 # number of subdivisions for each patch to create cut path

def delta_angle_at_r(r, patch_width):
    theta = np.arccos(1 - ((patch_width ** 2) / (2 * (r ** 2))))
    return theta

def get_mesh_r_at_angle(angle, height):
    # Create a ray at (0, 0, height) and cast it out
    ray_origin = np.array([0, 0, height])
    ray_direction = np.array([np.cos(angle), np.sin(angle), 0])

    r = 0
    locations, _, _ = surf.ray.intersects_location(ray_origins=[ray_origin], ray_directions=[ray_direction])
    if len(locations) > 0:
        # Find the radius of each intersection point and take the largest
        locations_r = np.linalg.norm(locations[:, :2], axis=1)
        r = locations_r.max()

    return r

def angle_height_step(height, angle):
    # At a given angle, we next split into height steps next.
    # At each height, the blade will cut a certain number of patches
    # around the "circumference" of the plant where the number depends
    # on the length of that "circumference"

    # To do it, we iterate around the ring at the height, and use the distance
    # From the center to determine the angle delta that a patch there will
    # provide. After we reach all the way around, we will have placed all
    # the patches.
    angle = 0
    r = get_mesh_r_at_angle(angle, height)
    while angle < 2 * np.pi:
        # We previously found the radius at what is now the starting angle
        # of the patch, now find the radius at the ending angle of the patch
        # If we start cutting the patch centered at the starting angle, then
        # the ending angle we will use will be this angle + the half of the
        # ange delta calculated from the radius of this patch.
        r = get_mesh_r_at_angle(angle, height)
        delta_ang = delta_angle_at_r(r, patch_width)
        end_angle = angle + delta_ang/2

        # Register this patch to the cut list

        angle = end_angle

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
axes = tm.creation.axis(axis_length=12.0)
scene.add_geometry(axes)
scene.show()
