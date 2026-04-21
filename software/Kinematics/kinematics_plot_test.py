import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

# -----------------------------
# Initial values
# -----------------------------
d = 500
ty = 100
tz = -45
theta_b0 = 0
theta_c0 = 0
zt0 = 0
yt0 = 0

# -----------------------------
# Kinematics function
# -----------------------------
def compute_xyz(d, ty, tz, theta_b, theta_c, zt, yt):

    x = d*np.sin(theta_c) - np.sin(theta_c)*(ty*np.cos(theta_b) - tz*np.sin(theta_b) + yt)
    y = -d*np.cos(theta_c) + np.cos(theta_c)*(ty*np.cos(theta_b) - tz*np.sin(theta_b) + yt) + d
    z = zt + tz*np.cos(theta_b) + ty*np.sin(theta_b)

    return x, y, z

# Initial point
x0, y0, z0 = compute_xyz(d, ty, tz, theta_b0, theta_c0, zt0, yt0)

# -----------------------------
# Figure + 3D plot
# -----------------------------
fig = plt.figure(figsize=(10, 10))

ax3d = fig.add_subplot(221, projection='3d')
ax_xy = fig.add_subplot(222)
ax_xz = fig.add_subplot(223)
ax_yz = fig.add_subplot(224)
plt.subplots_adjust(left=0.1, bottom=0.45)

p3d = ax3d.scatter(x0, y0, z0, c='red', s=100)

p_xy, = ax_xy.plot(x0, y0, 'ro', markersize=8)
p_xz, = ax_xz.plot(x0, z0, 'ro', markersize=8)
p_yz, = ax_yz.plot(y0, z0, 'ro', markersize=8)
coord_text = ax3d.text2D(
    0.02, 0.95,
    f"x = {x0:.2f}\ny = {y0:.2f}\nz = {z0:.2f}",
    transform=ax3d.transAxes
)

ax3d.set_xlabel('X')
ax3d.set_ylabel('Y')
ax3d.set_zlabel('Z')
ax3d.set_xlim(-500, 500)
ax3d.set_ylim(0, 1000)
ax3d.set_zlim(0, 1000)
ax3d.set_box_aspect([1, 1, 1])

# XY View
ax_xy.set_xlabel('X')
ax_xy.set_ylabel('Y')
ax_xy.set_xlim(-500, 500)
ax_xy.set_ylim(0, 1000)
ax_xy.grid(True)
ax_xy.set_aspect('equal', adjustable='box')

# XZ View
ax_xz.set_xlabel('X')
ax_xz.set_ylabel('Z')
ax_xz.set_xlim(-500, 500)
ax_xz.set_ylim(-50, 1000)
ax_xz.grid(True)
ax_xz.set_aspect('equal', adjustable='box')

# YZ View
ax_yz.set_xlabel('Y')
ax_yz.set_ylabel('Z')
ax_yz.set_xlim(0, 1000)
ax_yz.set_ylim(-50, 1000)
ax_yz.grid(True)
ax_yz.set_aspect('equal', adjustable='box')

# -----------------------------
# Slider axes
# -----------------------------
ax_theta_b = plt.axes([0.15, 0.23, 0.7, 0.03])
ax_theta_c = plt.axes([0.15, 0.19, 0.7, 0.03])
ax_zt      = plt.axes([0.15, 0.15, 0.7, 0.03])
ax_yt      = plt.axes([0.15, 0.11, 0.7, 0.03])

# -----------------------------
# Sliders
# Angles are in radians here
# -----------------------------
s_theta_b = Slider(ax_theta_b, 'Revolute', -np.pi/2, np.pi/2, valinit=theta_b0)
s_theta_c = Slider(ax_theta_c, 'Turntable', 0, 2*np.pi, valinit=theta_c0)  # avoid tan blowing up near pi/2
s_zt = Slider(ax_zt, 'Gantry', 0, 813, valinit=zt0)
s_yt = Slider(ax_yt, 'Extruder', 0, 295, valinit=yt0)

# -----------------------------
# Update function
# -----------------------------
def update(val):
    theta_b = s_theta_b.val
    theta_c = s_theta_c.val
    zt = s_zt.val
    yt = s_yt.val

    x, y, z = compute_xyz(d, ty, tz, theta_b, theta_c, zt, yt)

    # update scatter point
    # update 3D point
    p3d._offsets3d = ([x], [y], [z])

    # update 2D projections
    p_xy.set_data([x], [y])
    p_xz.set_data([x], [z])
    p_yz.set_data([y], [z])
    coord_text.set_text(
    f"x = {x:.2f}\ny = {y:.2f}\nz = {z:.2f}"
)

    fig.canvas.draw_idle()

# Connect sliders to update function
s_theta_b.on_changed(update)
s_theta_c.on_changed(update)
s_zt.on_changed(update)
s_yt.on_changed(update)

# Disc parameters
xc = 0
yc = 500
zc = 0
R = 150

theta = np.linspace(0, 2*np.pi, 100)
r = np.linspace(0, R, 30)

Theta, Radius = np.meshgrid(theta, r)

Xdisc = xc + Radius * np.cos(Theta)
Ydisc = yc + Radius * np.sin(Theta)
Zdisc = np.full_like(Xdisc, zc)

ax3d.plot_surface(Xdisc, Ydisc, Zdisc, alpha=0.5)
theta = np.linspace(0, 2*np.pi, 200)

x_circle = xc + R * np.cos(theta)
y_circle = yc + R * np.sin(theta)

ax_xy.plot(x_circle, y_circle, 'b-')

plt.show()