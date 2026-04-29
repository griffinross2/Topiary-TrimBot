import numpy as np
import trimesh as tm
from file_receiver import file_receiver_get_model_file_name

MODEL_EXTRA_SCALE = 1

plant_mesh = None
model_mesh = None

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

def calc_final_meshes():
    # Take the plant mesh after scanning, and the model mesh,
    # perform fitting and moving and produce the final working
    # meshes
    global plant_mesh
    global model_mesh

    plant_mesh = tm.load("meshes/plant.ply")
    model_mesh = tm.load(file_receiver_get_model_file_name())

    plant_mesh = scale_mesh_to_m(39.3701, plant_mesh) # Currently inches
    model_mesh = fit_mesh(model_mesh, plant_mesh)

def get_final_meshes():
    # Take the plant mesh after scanning, and the model mesh,
    # perform fitting and moving and produce the final working
    # meshes
    global plant_mesh
    global model_mesh

    return plant_mesh, model_mesh
