from toolpath.tool_path_voxel import *
from toolpath.gcode_generation import *
from toolpath.fitting import scale_mesh_to_m, fit_mesh
import trimesh as tm
import pyvista as pv
import numpy as np

def do_toolpath():
    plant_mesh = tm.load("meshes/plant.ply")
    model_mesh = tm.load("meshes/cube.obj")

    plant_mesh = scale_mesh_to_m(39.3701, plant_mesh)
    model_mesh = fit_mesh(model_mesh, plant_mesh)

    paths = []
    paths, _, _ = get_toolpath(plant_mesh, model_mesh, 0, 90, False)

    transform_data(paths)
    generate_gcode(paths, "gcode/out.gcode")


if __name__ == "__main__":
    do_toolpath()