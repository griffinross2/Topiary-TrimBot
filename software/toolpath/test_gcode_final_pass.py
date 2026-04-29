from tool_path_voxel import *
from gcode_generation import *
from fitting import scale_mesh_to_m, fit_mesh
import trimesh as tm
import pyvista as pv
import numpy as np
import matplotlib.pyplot as plt

def do_toolpath():
    plant_mesh = tm.load("meshes/plant.ply")
    model_mesh = tm.load("meshes/cube.obj")

    plant_mesh = scale_mesh_to_m(39.3701, plant_mesh)
    model_mesh = fit_mesh(model_mesh, plant_mesh)

    paths = []
    paths, _, _ = get_toolpath(plant_mesh, model_mesh, 0, 360, True)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    for path in paths:
        ax.plot([point[0] for point in path], [point[1] for point in path], [point[2] for point in path])
    
    plt.show()

    transform_data(paths)
    generate_gcode(paths, "gcode/out.gcode")

if __name__ == "__main__":
    do_toolpath()