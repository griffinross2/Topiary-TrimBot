import numpy as np
import trimesh as tm

MODEL_EXTRA_SCALE = 1

def fit_mesh(mesh, surf):
      # center both meshes
    translation = -surf.centroid
    translation[2] = 0
    surf.apply_translation(translation)
    mesh.apply_translation(-mesh.centroid)
    mesh.apply_translation((0, 0, surf.centroid[2]))

    # scale mesh larger than surface
    mesh_scale = (surf.extents / mesh.extents).min()
    mesh.apply_scale(mesh_scale)

    # scales and rotations
    scales = np.logspace(0,-2,50)
    rotations = np.linspace(0.0,360.0,50) # degrees

    # fitting operation
    for scale in scales:
        # scale
        s = np.diag([scale, scale, scale, 1])

        for rotation in rotations:
            # copy mesh
            mesh_test = mesh.copy()
        
            # rotation
            ang = np.radians(rotation)
            r_z = tm.transformations.rotation_matrix(ang, [0,0,1])
        
            # apply translation
            trans = r_z @ s
            mesh_test.apply_transform(trans)

            # simple check if all vertices of the mesh are inside the plant surface
            points_inside = surf.contains(mesh_test.vertices)
            is_inside = np.all(points_inside)

        if (is_inside):
            break
    
    if (not is_inside):
        raise RuntimeError("No fits")
    else:
        print(f"Found fit with scale {scale} and rotation {rotation}")
        return mesh_test
    
def scale_mesh_to_m(current_units_per_m, mesh):
    scale_factor = 1 / current_units_per_m
    mesh.apply_scale(scale_factor * MODEL_EXTRA_SCALE)
    return mesh