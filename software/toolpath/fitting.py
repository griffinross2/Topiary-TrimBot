import numpy as np
import trimesh as tm

MODEL_EXTRA_SCALE = 1.2

def fit_mesh(mesh, surf):
    # center both meshes
    translation = -surf.centroid
    translation[0] = 0
    translation[1] = 0
    surf.apply_translation(translation)
    mesh.apply_translation(-mesh.centroid)
    # surf.show(flags={'axis': True, 'wireframe': True})
    # mesh.show(flags={'axis': True, 'wireframe': True})
    

    # scale mesh larger than surface
    mesh_scale = (surf.extents / mesh.extents).min()
    mesh.apply_scale(mesh_scale)

    # scales and rotations
    scales = np.linspace(2,0.01,30)
    # rotations = np.linspace(0.0,360.0,20) # degrees
    rotations = [0]
    # x_bound = surf.bounds[:,0]
    # y_bound = surf.bounds[:,1]
    z_bound = surf.bounds[:,2]
    translations = [[0, 0, z] for z in np.linspace(z_bound[0], z_bound[1], 10)]

    # fitting operation
    for scale in scales:
        # scale
        s = np.diag([scale, scale, scale, 1])
        for translation in translations:
            for rotation in rotations:
                # copy mesh
                mesh_test = mesh.copy()
            
                # rotation
                ang = np.radians(rotation)
                r_z = tm.transformations.rotation_matrix(ang, [0,0,1], translation)
            
                # apply translation
                trans = r_z @ s
                mesh_test.apply_translation(translation)
                mesh_test.apply_transform(trans)

                # simple check if all vertices of the mesh are inside the plant surface
                points_inside = surf.contains(mesh_test.vertices)
                is_inside = np.all(points_inside)
                
                if is_inside:
                    break

            if is_inside:
                break

        if is_inside:
            break
    
    if not is_inside:
        raise RuntimeError("No fits")
    else:
        print(f"Found fit with scale {scale}, rotation {rotation} and translation {translation}")
        mesh_test.apply_scale(MODEL_EXTRA_SCALE)
        return mesh_test
    
def scale_mesh_to_m(current_units_per_m, mesh):
    scale_factor = 1 / current_units_per_m
    mesh.apply_scale(scale_factor)
    return mesh