from tool_path_voxel import *
from gcode_generation import *
import trimesh as tm
import pyvista as pv
import numpy as np

plant_mesh = tm.load("plant.ply")
model_mesh = tm.load("cube.obj")

paths = []
paths = get_toolpath(plant_mesh, model_mesh)

transform_data(paths)
generate_gcode(paths)
