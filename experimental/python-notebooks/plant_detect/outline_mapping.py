import pyvista as pv
import numpy as np
import io
from PIL import Image
import cairosvg

def generate_lumpy_sphere(radius=12.0, lumpy_factor=0.07, resolution=100):
    """Generate a lumpy sphere mesh."""
    # Create a sphere
    sphere = pv.Sphere(radius=radius, theta_resolution=resolution, phi_resolution=resolution)
    
    # Get the points of the sphere
    points = sphere.points
    
    # Apply lumpy deformation
    for i in range(points.shape[0]):
        x, y, z = points[i]
        theta = np.arctan2(y, x)
        phi = np.arccos(z / radius)
        noise = lumpy_factor * np.sin(3.123 * phi) * np.cos(5.278 * theta)
        scale = 1 + noise
        points[i] = points[i] * scale
    
    # Update the points of the sphere
    sphere.points = points
    
    return sphere

sphere = generate_lumpy_sphere()
plotter = pv.Plotter()
plotter.window_size = (1000, 800)
plotter.disable_parallel_projection()
plotter.camera.view_angle = 140
plotter.add_mesh(sphere, color='green', show_edges=False)
# plotter.show()

def retrieve_outline(mesh, angle: int, angle_step: int):
    r = 18.0
    d_stereo = 4.5

    new_mesh = mesh.rotate_z(angle_step, point=(0,0,0), inplace=True)
    
    plotter = pv.Plotter(off_screen=True)
    plotter.window_size = (1000, 800)
    plotter.disable_parallel_projection()
    plotter.camera.up = (0, 0, 1)
    plotter.camera.view_angle = 120
    plotter.add_mesh(new_mesh, color='black', show_edges=False)

    plotter.camera.position = (r, -d_stereo/2, 0)
    plotter.camera.focal_point = (0, -d_stereo/2, 0)
    plotter.save_graphic(f'outlines/img_{angle:03d}_right.svg')
    plotter.camera.position = (r, d_stereo/2, 0)
    plotter.camera.focal_point = (0, d_stereo/2, 0)
    plotter.save_graphic(f'outlines/img_{angle:03d}_left.svg')

def get_disparity_level(image_left: Image.Image, image_right: Image.Image):
    # just shift the left to the right up to a limit until a maximum correlation is found
    max_shift = 200
    best_shift = 0
    best_correlation = -1.0
    left_array = np.array(image_left.convert('1'))
    right_array = np.array(image_right.convert('1'))
    
    for shift in range(max_shift):
        correlation = np.sum(np.multiply(left_array, right_array))
        left_array = np.roll(left_array, 1, axis=1)
        if correlation > best_correlation:
            best_correlation = correlation
            best_shift = shift

    return best_shift

def disp_to_depth(disparity, image_width=1000, d_stereo=4.5, fov=120):
    depth = (image_width * d_stereo) / (2 * disparity * np.tan(np.radians(fov / 2)))
    return depth

def get_point_on_outline(image_left: Image.Image, angle):
    # Binary search for the border of black and white
    width, height = image_left.size
    r_interval = [0, width - 1]
    while r_interval[1] - r_interval[0] > 1:
        r_mid = (r_interval[0] + r_interval[1]) // 2
        x_mid = int(width // 2 + r_mid * np.cos(np.radians(angle)))
        y_mid = height - int(height // 2 + r_mid * np.sin(np.radians(angle)))

        # if we are outside of the image, take the inner interval and continue
        if x_mid < 0 or x_mid >= width or y_mid < 0 or y_mid >= height:
            r_interval[1] = r_mid
            continue

        pixel = image_left.getpixel((x_mid, y_mid))
        if pixel == (0,0,0):  # black
            r_interval[0] = r_mid
        else:  # white
            r_interval[1] = r_mid

    r_final = r_interval[1]
    x_final = width // 2 + r_final * np.cos(np.radians(angle))
    y_final = height - (height // 2 + r_final * np.sin(np.radians(angle)))
    return (x_final, y_final)

def project_outline_to_3d(outline_points, camera_dist, depth, angle, fov=120):
    width, height = 1000, 800
    x_scale = 2 * depth * np.tan(np.radians(120 / 2)) / width
    y_scale = 2 * depth * np.tan(np.radians(120 / 2)) / width
    r = camera_dist - depth

    # pick basis vectors
    n = np.array([np.cos(np.radians(angle)), np.sin(np.radians(angle)), 0])
    u1 = (0, 0, 1)
    u2 = np.cross(n, u1)
    R = np.array([u2, u1, n]).T

    points_3d = []
    for (x_2d, y_2d) in outline_points:
        # First get the offsets from the center
        x_offset = x_2d - width / 2 + 53
        y_offset = y_2d - height / 2

        # Scale to inches
        x_inch = x_offset * x_scale
        y_inch = y_offset * y_scale

        # Transform this cross section by placing it on its plane in 3D
        point_3d = np.matmul(R, np.array([x_inch, y_inch, 0])) + (r * n)

        points_3d.append(point_3d)

    return points_3d

def process_angle(angle, angle_step):
    retrieve_outline(sphere, angle, angle_step=angle_step)
    left_png = io.BytesIO()
    right_png = io.BytesIO()
    cairosvg.svg2png(url=f"outlines/img_{angle:03d}_left.svg", write_to=left_png, output_width=1000, output_height=800)
    cairosvg.svg2png(url=f"outlines/img_{angle:03d}_right.svg", write_to=right_png, output_width=1000, output_height=800)

    left_png.seek(0)
    right_png.seek(0)

    left_image = Image.open(left_png)
    right_image = Image.open(right_png)

    disparity = get_disparity_level(left_image, right_image)
    depth = disp_to_depth(disparity)
    print(f"Disp: {disparity:03d} pixels, depth: {depth:0.2f} in, offset: {18.0 - depth:0.2f} in")

    outline_points = []
    for a in range(0, 360, 10):
        point = get_point_on_outline(left_image, a)
        outline_points.append(point)

    points_3d = project_outline_to_3d(outline_points, 18.0, depth, angle)
    # for p in points_3d:
    #     print(f"Point: x={p[0]:0.2f}, y={p[1]:0.2f}, z={p[2]:0.2f}")

    return points_3d
    

points_3d = []

angle_step = 10
for angle in range(0, 360, angle_step):
    points_3d.extend(process_angle(angle, angle_step))


# Plot the points from our reconstruction
plotter = pv.Plotter(shape=(1,2))
plotter.subplot(0,1)
plotter.camera.position = (100, 0, 0)

# for p in points_3d:
#     sphere_pt = pv.Sphere(radius=0.2, center=p)
#     plotter.add_mesh(sphere_pt, color='red', show_edges=False)
point_cloud = pv.PolyData(np.array(points_3d))
reconstructed_mesh = point_cloud.reconstruct_surface()
plotter.add_mesh(reconstructed_mesh, color='red', show_edges=False)

# Plot the original lumpy sphere for reference
plotter.subplot(0,0)
plotter.camera.position = (1, 0, 0)
plotter.add_mesh(sphere, color='green', show_edges=False)

plotter.show()