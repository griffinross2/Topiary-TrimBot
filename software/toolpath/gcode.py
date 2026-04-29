from toolpath.tool_path_voxel import *
from toolpath.gcode_generation import *
from mesh_fitter import get_final_meshes
import trimesh as tm
import pyvista as pv
import numpy as np

def do_toolpath():
    plant_mesh, model_mesh = get_final_meshes()
    paths = []
    paths, _, _ = get_toolpath(plant_mesh, model_mesh, 0, 360, False)

    transform_data(paths)
    generate_gcode(paths, "gcode/out.gcode")


if __name__ == "__main__":
    do_toolpath()
