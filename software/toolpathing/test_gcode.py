from tool_path_voxel import *
import trimesh as tm
import pyvista as pv
import numpy as np

cloud = tm.points.PointCloud(pv.read("point_cloud.vtk").points)
plant_mesh = cloud.convex_hull
model_mesh = tm.load("cube-octahedron-compound.stl")

paths = []
paths = get_toolpath(plant_mesh, model_mesh)
print(paths[0])
