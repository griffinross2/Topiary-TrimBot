import trimesh as tm
import open3d as o3d

mesh = tm.load("cube-octahedron-compound.stl")
mesh.show()

cloud = tm.PointCloud(mesh.vertices)
new_mesh = cloud.convex_hull

new_mesh.show()