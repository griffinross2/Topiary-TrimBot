import trimesh as tm
import pyvista as pv
import numpy as np

CUTTER_WIDTH = 0.1
VOXEL_SIZE = CUTTER_WIDTH / 5
VOXEL_VICTIM_RANGE = CUTTER_WIDTH / 3
MODEL_EXTRA_SCALE = 1
STARTING_ANGLE_STEP = 8
ENDING_ANGLE_STEP = 18
POT_HEIGHT = 0.254
POT_DIAMETER = 0.28
PLANT_KEEPOUT_FRACTION = (2/3)


def fit_mesh(mesh, surf):
    # center both meshes
    mesh.apply_translation(-mesh.centroid)
    surf.apply_translation(-surf.centroid)
    # TODO: center mesh to surf centroid

    # scale mesh larger than surface
    mesh_scale = (surf.extents / mesh.extents).min()
    mesh.apply_scale(mesh_scale)

    # scales and rotations
    scales = np.logspace(0,-2,50)
    rotations = np.linspace(0.0,360.0,50) # degrees

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
    mesh.apply_scale(scale_factor * MODEL_EXTRA_SCALE)
    return mesh

def get_voxel_difference(voxel1, voxel2):
    diff = np.logical_and(voxel1.matrix, np.logical_not(voxel2.matrix))
    voxel_diff = tm.voxel.VoxelGrid(encoding=diff, transform=voxel1.transform)

    return voxel_diff

def show_voxel(voxel):
    voxel_mesh = voxel.as_boxes()
    voxel_mesh.show()

# Now we need to figure out how to get the cutter through each voxel.
# One approach we could take is to iterate a ray outward from the
# center at the height of each voxel layer. We will intersect the outermost
# voxel at each height. Then we make a path between each of these and move
# the cutter along it
def get_center_voxel(voxels):
    return np.floor((-voxels.translation)/VOXEL_SIZE)

def get_outmost_at_height(voxels, angle, height_index):
    start = get_center_voxel(voxels)
    start[2] = height_index
    angle = np.deg2rad(angle)
    last_voxel = start

    # We will now propagate a ray outward by picking points corresponding
    # to the closest voxel along the line.
    # To avoid steep slopes, we will check te angle range to decide whether
    # to use a function y=f(x) or x=f(y).
    # [0,pi/4], [3pi/4, 5pi/4], [7pi/4, 2pi] -> y=f(x)
    # (pi/4, 3pi/4), (5pi/4, 7pi/4) -> x=f(y)
    func_x = (angle <= np.pi/4) or (angle >= 3*np.pi/4 and angle <= 5*np.pi/4) or (angle >= 7*np.pi/4)
    if func_x:
        x_inc = 1 if (angle < np.pi/2) or (angle > 3*np.pi/2) else -1
        x = start[0]
        y = start[1]
        while int(x) >= 0 and int(x) < voxels.matrix.shape[0] and int(y) >= 0 and int(y) < voxels.matrix.shape[1]:
            if voxels.matrix[int(x), int(y), height_index]:
                last_voxel = (int(x), int(y), height_index)

            x += x_inc
            y += np.tan(angle)*x_inc
    else:
        y_inc = 1 if (angle < np.pi) else -1
        x = start[0]
        y = start[1]
        while int(x) >= 0 and int(x) < voxels.matrix.shape[0] and int(y) >= 0 and int(y) < voxels.matrix.shape[1]:
            if voxels.matrix[int(x), int(y), height_index]:
                last_voxel = (int(x), int(y), height_index)

            x += 1/(np.tan(angle))*y_inc
            y += y_inc

    return last_voxel
    
def move_dist_at_angle(voxels, start, dist, angle):
    # We will now propagate a ray outward by picking points corresponding
    # to the closest voxel along the line.
    # To avoid steep slopes, we will check te angle range to decide whether
    # to use a function y=f(x) or x=f(y).
    # [0,pi/4], [3pi/4, 5pi/4], [7pi/4, 2pi] -> y=f(x)
    # (pi/4, 3pi/4), (5pi/4, 7pi/4) -> x=f(y)
    dist_so_far = 0
    last_pos = start
    func_x = (angle <= np.pi/4) or (angle >= 3*np.pi/4 and angle <= 5*np.pi/4) or (angle >= 7*np.pi/4)
    if func_x:
        x_inc = 1 if (angle < np.pi/2) or (angle > 3*np.pi/2) else -1
        x = start[0]
        y = start[1]
        while int(x) >= 0 and int(x) < voxels.matrix.shape[0] and int(y) >= 0 and int(y) < voxels.matrix.shape[1] and dist_so_far < dist:
            last_pos = (int(x), int(y), int(start[2]))
            x += x_inc
            y += np.tan(angle)*x_inc

            dist_so_far += np.abs(1/np.cos(angle))
    else:
        y_inc = 1 if (angle < np.pi) else -1
        x = start[0]
        y = start[1]
        while int(x) >= 0 and int(x) < voxels.matrix.shape[0] and int(y) >= 0 and int(y) < voxels.matrix.shape[1] and dist_so_far < dist:
            last_pos = (int(x), int(y), int(start[2]))
            x += 1/(np.tan(angle))*y_inc
            y += y_inc

            dist_so_far += np.abs(1/np.sin(angle))

    return last_pos

def kill_through_path(voxels, start, angle):
    # We will now propagate a ray outward by picking points corresponding
    # to the closest voxel along the line.
    # To avoid steep slopes, we will check te angle range to decide whether
    # to use a function y=f(x) or x=f(y).
    # [0,pi/4], [3pi/4, 5pi/4], [7pi/4, 2pi] -> y=f(x)
    # (pi/4, 3pi/4), (5pi/4, 7pi/4) -> x=f(y)
    func_x = (angle <= np.pi/4) or (angle >= 3*np.pi/4 and angle <= 5*np.pi/4) or (angle >= 7*np.pi/4)
    if func_x:
        x_inc = 1 if (angle < np.pi/2) or (angle > 3*np.pi/2) else -1
        x = start[0]
        y = start[1]
        while int(x) >= 0 and int(x) < voxels.matrix.shape[0] and int(y) >= 0 and int(y) < voxels.matrix.shape[1]:
            # print(f"killing: {int(x):d}, {int(y):d}, {int(start[2]):d}")
            voxels.matrix[int(x), int(y), int(start[2])] = False
            # plant_voxel.matrix[int(x), int(y), int(start[2])] = False
            x += x_inc
            y += np.tan(angle)*x_inc
    else:
        y_inc = 1 if (angle < np.pi) else -1
        x = start[0]
        y = start[1]
        while int(x) >= 0 and int(x) < voxels.matrix.shape[0] and int(y) >= 0 and int(y) < voxels.matrix.shape[1]:
            # print(f"killing: {int(x):d}, {int(y):d}, {int(start[2]):d}")
            voxels.matrix[int(x), int(y), int(start[2])] = False
            # plant_voxel.matrix[int(x), int(y), int(start[2])] = False
            x += 1/(np.tan(angle))*y_inc
            y += y_inc

def kill_other_victims(voxels, angle, last_voxel):
    victim_range_voxels = int(VOXEL_VICTIM_RANGE/VOXEL_SIZE)
    # center = get_center_voxel(voxels)
    # last_voxel_dist = np.linalg.norm(last_voxel[0:2])
    start = last_voxel
    angle = np.deg2rad(angle)
    for i in range(-victim_range_voxels, victim_range_voxels+1):
        new_angle = angle + np.sign(i) * np.pi / 2
        if new_angle < 0:
            new_angle += np.pi*2
        if new_angle > np.pi*2:
            new_angle -= np.pi*2
        new_start = move_dist_at_angle(voxels, start, np.abs(i), new_angle)
        # print(angle, start, new_start, i)
        kill_through_path(voxels, new_start, angle)
        
def get_cut_path(voxels, angle):
    path = []
    for i in range(voxels.matrix.shape[2]):
        voxel_at_i = get_outmost_at_height(voxels, angle, i)
        point = voxels.indices_to_points(np.array([voxel_at_i]))[0]
        # Reject points that may have crossed to the other side
        # of the plant
        if angle >= 0 and angle < 90:
            if point[0] > 0 and point[1] > 0:
                path.append(point)
        elif angle >= 90 and angle < 180:
            if point[0] < 0 and point[1] > 0:
                path.append(point)
        elif angle >= 180 and angle < 270:
            if point[0] < 0 and point[1] < 0:
                path.append(point)
        elif angle >= 270 and angle <= 360:
            if point[0] > 0 and point[1] < 0:
                path.append(point)

        # print(i, angle, voxel_at_i)
        # Remove them as we go
        # mat = voxels.matrix
        # mat[np.array(voxel_at_i, dtype=int)] = False
        # voxels = tm.voxel.VoxelGrid(encoding=mat, transform=voxels.transform)
        voxels.matrix[int(voxel_at_i[0]), int(voxel_at_i[1]), int(voxel_at_i[2])] = False
        # plant_voxel.matrix[int(voxel_at_i[0]), int(voxel_at_i[1]), int(voxel_at_i[2])] = False

        # Remove other voxels that the cutting blade should be passing through/under
        # kill_other_victims(voxels, angle, voxel_at_i)

    return voxels, path


# show_voxel(diff_voxel)
# print(diff_voxel.matrix.shape)

def remove_points_in_keepout(plant_mesh, paths):
    keepout_radius = POT_DIAMETER / 2
    plant_top = plant_mesh.bounds[1][2]
    keepout_height = PLANT_KEEPOUT_FRACTION * plant_top

    new_paths = []
    for path in paths:
        new_path = []
        for point in path:
            point_r = np.linalg.norm((point[0], point[1]))
            point_z = point[2]
            if point_r > keepout_radius or point_z > keepout_height:
                new_path.append(point)
        new_paths.append(new_path)

    return new_paths

def trim_dumb_paths(paths):
    new_paths = []

    for path in paths:
        if len(path) >= 2:
            new_paths.append(path)

    return new_paths

def get_toolpath(plant_mesh, model_mesh):
    plant_mesh = scale_mesh_to_m(39.3701, plant_mesh) # Currently inches
    model_mesh = fit_mesh(model_mesh, plant_mesh)

    bounds = plant_mesh.bounds

    plant_voxel = plant_mesh.voxelized(pitch=VOXEL_SIZE, method='binvox', bounds=bounds)
    model_voxel = model_mesh.voxelized(pitch=VOXEL_SIZE, method='binvox', bounds=bounds)
    plant_voxel.fill()
    model_voxel.fill()

    diff_voxel = get_voxel_difference(plant_voxel, model_voxel)

    paths = []
    num_passes = int(np.max(diff_voxel.matrix.shape)/2)
    for p in range(num_passes):
        angle_lerp = int(((p/num_passes) * (ENDING_ANGLE_STEP-STARTING_ANGLE_STEP)) + STARTING_ANGLE_STEP)
        for a in range(0, 360, angle_lerp):
            diff_voxel, path = get_cut_path(diff_voxel, a)
            paths.append(path)

    # show_voxel(diff_voxel)
    # show_voxel(plant_voxel)
    paths = remove_points_in_keepout(plant_mesh, paths)
    paths = trim_dumb_paths(paths)
    return paths

if __name__ == "__main__":
    cloud = tm.points.PointCloud(pv.read("point_cloud.vtk").points)
    plant_mesh = cloud.convex_hull
    model_mesh = tm.load("cube-octahedron-compound.stl")

    paths = get_toolpath(plant_mesh, model_mesh)
    print(len(paths))