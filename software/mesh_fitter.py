import numpy as np
import trimesh as tm
from file_receiver import file_receiver_get_model_file_name
from toolpath.fitting import scale_mesh_to_m, fit_mesh

plant_mesh = None
model_mesh = None

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
