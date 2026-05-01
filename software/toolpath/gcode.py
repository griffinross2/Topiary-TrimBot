from toolpath.tool_path_voxel import *
from toolpath.gcode_generation import *
from toolpath.fitting import scale_mesh_to_m, fit_mesh
from mesh_fitter import get_final_meshes
import trimesh as tm
import pyvista as pv
import numpy as np
import matplotlib.pyplot as plt

def do_toolpath(plant_mesh=None, model_mesh=None):
    if plant_mesh is None or model_mesh is None:
        plant_mesh, model_mesh = get_final_meshes()
    paths = []
    paths, _, _ = get_toolpath(plant_mesh, model_mesh, 0, 360, False)
    
    # height = -0.05
    # fig = plt.figure()
    # ax = fig.add_subplot(111, projection='3d')
    # for path in paths:
    #     path = np.array(path)
    #     points = path[np.abs(path[:, 2] - height) < 0.05]
    #     # points = path
    #     ax.plot(points[:, 0], points[:, 1], points[:, 2])
    
    # plt.show()

    paths = transform_data(paths)
    generate_gcode(paths, "gcode/out.gcode")


if __name__ == "__main__":
    plant_mesh = tm.load("meshes/plant.ply")
    model_mesh = tm.load("meshes/pyramid.stl")

    plant_mesh = scale_mesh_to_m(39.3701, plant_mesh)
    model_mesh = fit_mesh(model_mesh, plant_mesh)

    do_toolpath(plant_mesh, model_mesh)
