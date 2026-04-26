from toolpath.tool_path_voxel import *
from toolpath.gcode_generation import *
import trimesh as tm
import pyvista as pv
import numpy as np

def do_toolpath(mesh_path):
    plant_mesh = tm.load("meshes/plant.ply")
    model_mesh = tm.load(mesh_path)

    paths = []
    paths, _, _ = get_toolpath(plant_mesh, model_mesh, 0, 90, False)

    transform_data(paths)
    generate_gcode(paths, "gcode/out.gcode")


if __name__ == "__main__":
    do_toolpath("meshes/cube.obj")