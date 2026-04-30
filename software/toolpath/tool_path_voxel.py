import trimesh as tm
import pyvista as pv
import numpy as np

CUTTER_WIDTH = 0.1
VOXEL_SIZE = CUTTER_WIDTH / 4
VOXEL_VICTIM_RANGE = CUTTER_WIDTH / 3
STARTING_ANGLE_STEP = 9
ENDING_ANGLE_STEP = 16
SCAN_SET_HEIGHT = (400 / 1000)
# STARTING_ANGLE_STEP = 1
# ENDING_ANGLE_STEP = 1
POT_HEIGHT = 0.254
POT_DIAMETER = 0.28
PLANT_KEEPOUT_FRACTION = (1/2)
PLANT_KEEPOUT_DIAMETER = POT_DIAMETER * 0.75
R_SLOP = 0.003

def voxelize_meshes(plant_mesh, model_mesh, pitch=VOXEL_SIZE):
    # This function implemented by ChatGPT

    # 1. Compute combined bounding box
    bounds = np.vstack((plant_mesh.bounds, model_mesh.bounds))
    min_corner = bounds.min(axis=0)
    max_corner = bounds.max(axis=0)

    # 3. Compute grid dimensions
    grid_size = np.ceil((max_corner - min_corner) / pitch).astype(int)
    # print(grid_size)

    # 4. Define transform (voxel -> world)
    # This maps voxel indices to world coordinates
    transform = np.eye(4)
    transform[:3, :3] *= pitch
    transform[:3, 3] = min_corner
    # print(transform)

    # 5. Create empty occupancy grids
    vox_a = np.zeros(grid_size, dtype=bool)
    vox_b = np.zeros(grid_size, dtype=bool)

    # 6. Voxelize each mesh into the SAME grid
    v_a = plant_mesh.voxelized(pitch)
    v_b = model_mesh.voxelized(pitch)
    # v_b.as_boxes().show()

    # 7. Convert voxel indices into global grid indices
    def insert(vox, v):
        # local voxel indices
        pts = v.sparse_indices

        # convert to world coords
        world = tm.transform_points(pts, v.transform)

        # convert to global grid indices
        idx = np.floor((world - min_corner) / pitch).astype(int)

        # clip just in case
        idx = np.clip(idx, 0, np.array(vox.shape) - 1)

        # print(idx[0,:], idx[1,:], idx[2,:])
        vox[idx[:,0], idx[:,1], idx[:,2]] = True

    insert(vox_a, v_a)
    insert(vox_b, v_b)

    vg_a = tm.voxel.VoxelGrid(vox_a, transform=transform)
    vg_b = tm.voxel.VoxelGrid(vox_b, transform=transform)
    return vg_a, vg_b

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

def get_outmost_at_height(diff_voxels, model_voxels, angle, height_index):
    start = get_center_voxel(diff_voxels)
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
        while int(x) >= 0 and int(x) < diff_voxels.matrix.shape[0] and int(y) >= 0 and int(y) < diff_voxels.matrix.shape[1]:
            if diff_voxels.matrix[int(x), int(y), height_index]:
                last_voxel = (int(x), int(y), height_index)
            if model_voxels.matrix[int(x), int(y), height_index]:
                last_voxel = (int(x), int(y), height_index)


            x += x_inc
            y += np.tan(angle)*x_inc
    else:
        y_inc = 1 if (angle < np.pi) else -1
        x = start[0]
        y = start[1]
        while int(x) >= 0 and int(x) < diff_voxels.matrix.shape[0] and int(y) >= 0 and int(y) < diff_voxels.matrix.shape[1]:
            if diff_voxels.matrix[int(x), int(y), height_index]:
                last_voxel = (int(x), int(y), height_index)
            if model_voxels.matrix[int(x), int(y), height_index]:
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
        
def get_cut_path(voxels, angle, plant_voxel, model_voxel):
    path = []
    for i in range(voxels.matrix.shape[2]):
        voxel_at_i = get_outmost_at_height(voxels, model_voxel, angle, i)
        point = voxels.indices_to_points(np.array([voxel_at_i]))[0]
        # Clip points that may have crossed to the other side
        # of the plant
        # if angle >= 0 and angle < 45 or angle >= 315 and angle <= 360:
        #     if point[0] < 0:
        #         path.append([R_SLOP*np.cos(np.deg2rad(angle)), R_SLOP*np.sin(np.deg2rad(angle)), point[2]])
        #     else:
        #         path.append(point)
        # elif angle >= 45 and angle < 135:
        #     if point[1] < 0:
        #         path.append([R_SLOP*np.cos(np.deg2rad(angle)), R_SLOP*np.sin(np.deg2rad(angle)), point[2]])
        #     else:
        #         path.append(point)
        # elif angle >= 135 and angle < 225:
        #     if point[0] > 0:
        #         path.append([R_SLOP*np.cos(np.deg2rad(angle)), R_SLOP*np.sin(np.deg2rad(angle)), point[2]])
        #     else:
        #         path.append(point)
        # elif angle >= 225 and angle < 315:
        #     if point[1] > 0:
        #         path.append([R_SLOP*np.cos(np.deg2rad(angle)), R_SLOP*np.sin(np.deg2rad(angle)), point[2]])
        #     else:
        #         path.append(point)

        # Check if point is near zero or angle is more than 45 degrees away
        point_r = np.linalg.norm((point[0], point[1]))
        point_angle = np.arctan2(point[1], point[0])
        if point_angle < 0:
            point_angle += np.pi*2
        if point_r < R_SLOP or abs(point_angle - np.deg2rad(angle)) > np.deg2rad(45):
            # print(f"Clipping point {point} at angle {angle} and point angle {point_angle}")
            path.append([R_SLOP*np.cos(np.deg2rad(angle)), R_SLOP*np.sin(np.deg2rad(angle)), point[2]])
        else:
            path.append(point)

        # print(i, angle, voxel_at_i)
        # Remove them as we go
        # mat = voxels.matrix
        # mat[np.array(voxel_at_i, dtype=int)] = False
        # voxels = tm.voxel.VoxelGrid(encoding=mat, transform=voxels.transform)
        voxels.matrix[int(voxel_at_i[0]), int(voxel_at_i[1]), int(voxel_at_i[2])] = False
        plant_voxel.matrix[int(voxel_at_i[0]), int(voxel_at_i[1]), int(voxel_at_i[2])] = False

        # Remove other voxels that the cutting blade should be passing through/under
        # kill_other_victims(voxels, angle, voxel_at_i)

    return voxels, path, plant_voxel


# show_voxel(diff_voxel)
# print(diff_voxel.matrix.shape)

def remove_points_in_keepout(plant_mesh, paths):
    new_paths = []
    for path in paths:
        new_path = []
        for point in path:
            point_r = np.linalg.norm((point[0], point[1]))
            point_z = point[2]

            keepout_radius = POT_DIAMETER / 2 if (point_z + SCAN_SET_HEIGHT < POT_HEIGHT) else PLANT_KEEPOUT_DIAMETER / 2
            plant_top = plant_mesh.bounds[1][2]
            keepout_height = PLANT_KEEPOUT_FRACTION * plant_top

            if point_r > keepout_radius or point_z > keepout_height:
                new_path.append(point)
            # else:
            #     print(point)
        new_paths.append(new_path)

    return new_paths

def kill_voxels_outside_radius(voxels, radius):
    # Get rid of voxels that should have been trimmed
    for i in range(voxels.matrix.shape[0]):
        for j in range(voxels.matrix.shape[1]):
            center = voxels.indices_to_points(np.array([[i,j,0]]))[0]
            r = np.linalg.norm((center[0], center[1]))
            if r > radius:
                voxels.matrix[i, j, :] = False

def get_toolpath(plant_mesh, model_mesh, angle_start=0, angle_end=360, last_pass_only=False):
    plant_voxel, model_voxel = voxelize_meshes(plant_mesh, model_mesh)
    plant_voxel.fill()
    model_voxel.fill()

    diff_voxel = get_voxel_difference(plant_voxel, model_voxel)

    paths = []
    pass_start_indices = []
    max_radius = np.max([np.linalg.norm(plant_mesh.bounds[0][:2]), np.linalg.norm(plant_mesh.bounds[1][:2])])
    max_radius_voxels = max_radius / VOXEL_SIZE
    num_passes = int(max_radius_voxels)
    for p in range(num_passes):
        pass_start_len = len(paths)
        pass_start_indices.append(len(paths))
        angle_lerp = int(((p/num_passes) * (ENDING_ANGLE_STEP-STARTING_ANGLE_STEP)) + STARTING_ANGLE_STEP)
        for a in range(angle_start, angle_end, angle_lerp):
            diff_voxel, path, plant_voxel = get_cut_path(diff_voxel, a, plant_voxel, model_voxel)
            if len(path) >= 2:
                paths.append(path)
        if len(paths) <= pass_start_len:
            break
        kill_voxels_outside_radius(diff_voxel, max_radius - (p+1)*VOXEL_SIZE)

    # show_voxel(diff_voxel)
    # show_voxel(plant_voxel)
    if last_pass_only:
        paths = paths[pass_start_indices[-1]:]

    paths = remove_points_in_keepout(plant_mesh, paths)

    return paths, plant_voxel, model_voxel

if __name__ == "__main__":
    from fitting import scale_mesh_to_m, fit_mesh

    plant_mesh = tm.load("meshes/plant.ply")
    model_mesh = tm.load("meshes/cube.obj")
    
    plant_mesh = scale_mesh_to_m(39.3701, plant_mesh) # Currently inches
    model_mesh = fit_mesh(model_mesh, plant_mesh)

    paths, plant_voxel, model_voxel = get_toolpath(plant_mesh, model_mesh)
    plant_voxel_mesh = plant_voxel.as_boxes()
    # plant_voxel_mesh.visual.face_colors = [50, 200, 50, 255]
    # print(plant_voxel_mesh.bounds)
    plant_voxel_mesh.show()
    # model_voxel.as_boxes().show()
