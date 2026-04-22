import trimesh as tm
import matplotlib.pyplot as plt
import pyvista as pv
from tool_path_voxel import fit_mesh, scale_mesh_to_m
import numpy as np

# mesh = tm.load("cube-octahedron-compound.stl")
mesh = tm.load("pyramid.stl")
mesh.apply_translation(-mesh.centroid)

cloud = tm.points.PointCloud(pv.read("point_cloud.vtk").points)
plant = cloud.convex_hull

# Scale and fit
plant_mesh = scale_mesh_to_m(39.3701, plant) # Currently inches
model_mesh = fit_mesh(mesh, plant_mesh)
# plant_mesh = plant
# model_mesh = mesh

# To pyvista
model_mesh.export("mesh.ply")
model_mesh = pv.read('mesh.ply')

plant_mesh.export("plant.ply")
plant_mesh = pv.read('plant.ply')

def polydata_to_segments(mesh):
    segments = []

    i = 0
    while i < len(mesh.lines):
        for j in range(mesh.lines[i] - 1):
            segments.append([mesh.points[mesh.lines[i+j+1]], mesh.points[mesh.lines[i+j+2]]])

        i += mesh.lines[i] + 1

    return segments

def get_slice_segments(mesh, z):
    slice_mesh = mesh.slice(normal='z', origin=(0, 0, z))
    segments = polydata_to_segments(slice_mesh)

    return segments

def show_segments(segments, fmt, graph_size):
    plt.figure()
    for i in range(len(segments)):
        plt.xlim(-graph_size, graph_size)
        plt.ylim(-graph_size, graph_size)
        plt.plot([segments[i][0][0], segments[i][1][0]], [segments[i][0][1], segments[i][1][1]], fmt)

    plt.show()

bounds = plant_mesh.bounds
graph_extent = np.max(np.abs([bounds.x_min, bounds.x_max, bounds.y_min, bounds.y_max]))
for z in np.linspace(bounds.z_min, bounds.z_max, 20):
    model_segments = get_slice_segments(model_mesh, z)
    plant_segments = get_slice_segments(plant_mesh, z)
    show_segments(model_segments + plant_segments, 'k-', graph_extent)