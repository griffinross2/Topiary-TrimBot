import trimesh as tm
import matplotlib.pyplot as plt
import pyvista as pv

mesh = tm.load("cube-octahedron-compound.stl")
mesh.apply_translation(-mesh.centroid)
mesh.export("mesh.ply")
# mesh.show()

# cloud = tm.PointCloud(mesh.vertices)
# new_mesh = cloud.convex_hull

# new_mesh.show()

mesh = pv.read('mesh.ply')

slice_mesh = mesh.slice(normal='z', origin=(0, 0, 5))
plotter = pv.Plotter()
plotter.add_mesh(slice_mesh)
plotter.show()
# contour = slice_mesh.contour()

def polydata_to_segments(mesh):
    segments = []

    i = 0
    while i < len(mesh.lines):
        for j in range(mesh.lines[i] - 1):
            print(i, j)
            segments.append([mesh.points[mesh.lines[i+j+1]], mesh.points[mesh.lines[i+j+2]]])

        i += mesh.lines[i] + 1

    return segments

segments = polydata_to_segments(slice_mesh)

plt.figure()
for i in range(len(segments)):
    plt.plot([segments[i][0][0], segments[i][1][0]], [segments[i][0][1], segments[i][1][1]])

plt.show()
