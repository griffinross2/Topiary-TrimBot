import trimesh as tm
import pyvista as pv
import pymeshlab as pml
import numpy as np
import open3d as o3d

# import pynanoinstantmeshes as pynim

# pyvista point cloud reconstruction
cloud = tm.points.PointCloud(pv.read("point_cloud.vtk").points)
surf = cloud.convex_hull

# trimesh manipulation
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

            # simple check if all vertices of the mesh are inside the plant surface
            points_inside = surf.contains(mesh_test.vertices)
            is_inside = np.all(points_inside)

        if (is_inside):
            break
    
    if (not is_inside):
        raise RuntimeError("No fits")
    else:
        print(f"Found fit with scale {scale} and rotation {rotation}")
        return mesh_test
    
def scale_mesh_to_m(current_units_per_m, mesh):
    scale_factor = 1 / current_units_per_m
    mesh.apply_scale(scale_factor)
    return mesh

surf = scale_mesh_to_m(39.3701, surf) # Currently inches
mesh = fit_mesh(mesh,surf)

# create cut list
cut_width = 0.1 # m - effective width of cutting blade
cut_ang = 90 # degrees - angle of blade wrt cutting plane
cut_depth = 0.01 # m - depth of each cut pass
patch_height = 0.05 # m - height of patches used to cover the surface
patch_width = cut_width # m - width of patches used to cover the surface
patch_subdivisions = 10 # number of subdivisions for each patch to create cut path
r_plant_skip_threshold = 0.01 # m - if the plant radius is smaller than this, skip cutting there
completion_threshold = 0.005 # m - error within which an area cas been cut enough, must be less than r_plant_skip_threshold
ring_completion_distance_threshold = patch_height/2 # m - if a ring is this close to a completed ring, then we can consider it completed as well
# r_plant_skip_angle_step = np.deg2rad(15) # degrees - angle step when skipping due to small r
max_delta_angle = np.deg2rad(30) # degrees - maximum angle step for cutting
max_passes = 40 # maximum number of passes to attempt before giving up

# guard parameters
# We want to avoid cutting the stem of the plant. This means not cutting to deeply in from the sides,
# nor should we cut the bottom of the plant. Cutting the top is OK as long as we don't go too deep.
# Therefore, we will keepout of a cylinder that extends down infinitely and stops at a certain fraction
# of the plant height.
guard_radius_fraction = 0.3 # fraction of plant radius to keep out of
guard_height_fraction = 0.5 # fraction of plant height below which to keep out of if in the radius
plant_max_radius = max(np.linalg.norm(surf.bounds[0][:2]), np.linalg.norm(surf.bounds[1][:2]))
guard_radius = plant_max_radius * guard_radius_fraction
plant_height = surf.bounds[1][2] - surf.bounds[0][2]
plant_bottom = surf.bounds[0][2]
guard_height = plant_bottom + plant_height * guard_height_fraction

def delta_angle_at_r(r, patch_width):
    small_angle = patch_width / (2*r)
    if small_angle > max_delta_angle:
        return max_delta_angle
    else:
        return small_angle


def get_r_at_angle(mesh, angle, height):
    # Create a ray at (0, 0, height) and cast it out
    ray_origin = np.array([0, 0, height])
    ray_direction = np.array([np.cos(angle), np.sin(angle), 0])

    r = 0
    locations, _, _ = mesh.ray.intersects_location(ray_origins=[ray_origin], ray_directions=[ray_direction])
    if len(locations) > 0:
        # Find the radius of each intersection point and take the largest
        locations_r = np.linalg.norm(locations[:, :2], axis=1)
        r = locations_r.max()

    return r


def height_step(plant_mesh, model_mesh, height):
    print(f"Height: {height}")
    # At a given angle, we next split into height steps next.
    # At each height, the blade will cut a certain number of patches
    # around the "circumference" of the plant where the number depends
    # on the length of that "circumference"

    # To do it, we iterate around the ring at the height, and use the distance
    # From the center to determine the angle delta that a patch there will
    # provide. After we reach all the way around, we will have placed all
    # the patches. At the same time, we we will generate a new mesh from
    # where the patch will cut to.
    full_completion = True
    new_mesh_points = []
    angle = 0
    while angle < 2 * np.pi:
        # We previously found the radius at what is now the starting angle
        # of the patch, now find the radius at the ending angle of the patch
        # If we start cutting the patch centered at the starting angle, then
        # the ending angle we will use will be this angle + the half of the
        # ange delta calculated from the radius of this patch.

        r_plant = get_r_at_angle(plant_mesh, angle, height)
        # Skip if the plant radius is small
        if r_plant < r_plant_skip_threshold:
            # angle += r_plant_skip_angle_step
            # new_mesh_points.append([r_plant * np.cos(angle), r_plant * np.sin(angle), height])
            # continue
            break
            
        delta_ang = delta_angle_at_r(r_plant, patch_width)
        end_angle = angle + delta_ang

        # Now get r at the model mesh
        r_model = get_r_at_angle(model_mesh, angle, height)
        r_cut = min(r_plant - r_model, cut_depth)
        r_dest = r_plant - r_cut

        if abs(height - (-0.15851270371982906)) < 1e-4:
            print(f"Rdest: {r_dest}")

        # Check if we are cutting into the guard region, if so, skip this patch
        if r_dest < guard_radius and height < guard_height:
            new_mesh_points.append([r_plant * np.cos(angle), r_plant * np.sin(angle), height])
            angle = end_angle
            print(f"Skipping patch at angle {angle} and height {height} due to guard region")
            continue

        # Add new mesh point
        new_mesh_points.append([r_dest * np.cos(angle), r_dest * np.sin(angle), height])

        if r_cut < completion_threshold:
            # Skip because this is done
            angle = end_angle
            continue

        # If we are here then this height_step is not fully complete
        full_completion = False

        # Go to the angle of the next patch
        angle = end_angle

    # Return the new mesh points
    return new_mesh_points, full_completion

def height_step_mesh_only(plant_mesh, height):
    # Only build mesh, don't do any cutting. This is for when this height step is done.
    new_mesh_points = []
    angle = 0
    while angle < 2 * np.pi:
        r_plant = get_r_at_angle(plant_mesh, angle, height)
        if r_plant < r_plant_skip_threshold:
            break
        new_mesh_points.append([r_plant * np.cos(angle), r_plant * np.sin(angle), height])
        angle += delta_angle_at_r(r_plant, patch_width)

    return new_mesh_points

def cut_pass(plant_mesh, model_mesh, height_completions):
    full_completion = True

    # For a given pass, we will call height_step() for each height,
    # which will determine the cuts to be made around a ring of that height.
    heights = np.arange(plant_mesh.bounds[0][2], plant_mesh.bounds[1][2] + patch_height, patch_height)


    # Record new mesh points and cut path by iterating through height steps
    new_mesh_points = []
    for height in heights:
        # First, if rings adjacent to this height are already complete, then we can skip this height
        if len(height_completions) > 0 and np.any(np.abs(height_completions - height) <= ring_completion_distance_threshold):
            new_mesh_points.extend(height_step_mesh_only(plant_mesh, height))
            height_completions.append(height)
            continue

        height_mesh_points, height_full_completion = height_step(plant_mesh, model_mesh, height)
        new_mesh_points.extend(height_mesh_points)
        print(f"Height {height} full completion: {height_full_completion}")
        print(f"Plant min height: {plant_mesh.bounds[0][2]}, max height: {plant_mesh.bounds[1][2]}")
        print(f"Distance to closest height complete at start of pass: {np.min(np.abs(height_completions - height)) if len(height_completions) > 0 else 'N/A'}")

        if not height_full_completion:
            # Pass is not fully complete
            full_completion = False
        else:
            height_completions.append(height)

    # Reconstruct new surface
    new_cloud = tm.points.PointCloud(new_mesh_points)
    # new_cloud.show()
    new_plant_mesh = new_cloud.convex_hull

    return new_plant_mesh, full_completion, height_completions, new_cloud

surfs = [surf]
full_completion = False
height_completions = []
i = 0
while not full_completion and i < max_passes:
    print(f"Pass {i+1}")
    new_surf, full_completion, height_completions, new_cloud = cut_pass(surfs[i], mesh, height_completions)
    surfs.append(new_surf)
    i += 1

if i == max_passes and not full_completion:
    print(f"Failed to converge to model after {i-1} passes")
else:
    print(f"Completed in {i-1} passes")

for s in surfs[:-1]:
    # visualize
    s.visual.face_colors = [0, 255, 0, 150]
    scene = tm.Scene([s])
    axes = tm.creation.axis(axis_length=1.0)
    scene.add_geometry(axes)
    scene.show()
# visualize
# mesh.visual = tm.visual.ColorVisuals(mesh)
# mesh.visual.face_colors = [255, 0, 0, 150]
# surfs[-1].visual.face_colors = [0, 255, 0, 150]
# scene = tm.Scene([surfs[-1]])
# axes = tm.creation.axis(axis_length=1.0)
# scene.add_geometry(axes)
# scene.show()