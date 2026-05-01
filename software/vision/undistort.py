import numpy as np
from PIL import Image,ImageFilter
import cv2
import glob

def get_cropped_fov(new_camera_matrix, crop_x, crop_y, crop_width, crop_height):
    """
    Calculates the FOV for a cropped region of an undistorted image.
    """
    # 1. Update the Camera Matrix for the crop
    # We only shift the principal point (cx, cy) by the crop offset (x, y)
    K_crop = new_camera_matrix.copy()
    K_crop[0, 2] -= crop_x  # Adjusted cx
    K_crop[1, 2] -= crop_y  # Adjusted cy

    # 2. Extract focal lengths and adjusted principal points
    fx = K_crop[0, 0]
    fy = K_crop[1, 1]
    cx_prime = K_crop[0, 2]
    cy_prime = K_crop[1, 2]

    # 3. Calculate FOV using arctan of the distances to the edges
    # Horizontal: angle from cx to left edge + angle from cx to right edge
    fov_h = np.arctan2(cx_prime, fx) + np.arctan2(crop_width - cx_prime, fx)
    
    # Vertical: angle from cy to top edge + angle from cy to bottom edge
    fov_v = np.arctan2(cy_prime, fy) + np.arctan2(crop_height - cy_prime, fy)

    return np.degrees(fov_h), np.degrees(fov_v)

def undistort(img, pos):
    # undistort
    cv_img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)
    #img.show()

    try:
        # file = cv2.FileStorage("vision/parameters.yml", cv2.FileStorage_READ)
        params = np.load("vision/parameters.npz")
        # file = cv2.FileStorage("vision/parameters.yml", cv2.FileStorage_READ)
    except Exception as e:
        print("could not open calibration parameter files: " + e)
        raise RuntimeError("calibration failure")
    
    camera_matrix = params["camera_matrix"]
    dist_coeffs = params["dist_coeffs"]

    h, w = cv_img.shape[:2]
    new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(
        camera_matrix, dist_coeffs, (w, h), 1, (w, h)
    )
    x, y, w, h = roi
    
    undst = cv2.undistort(cv_img, camera_matrix, dist_coeffs, None, new_camera_matrix)

    # cv2.imshow('original', cv_img)
    # new_img = cv2.resize(undst, (1280, 720))
    # cv2.imshow('undistorted', new_img)
    # cv2.waitKey(0)

    # print(cv_img.shape)
    # print(x, y, w, h)
    undst = undst[y:y+h, x:x+w]

    # cv2.imshow('undistorted', undst)
    # cv2.waitKey(0)
    rgb_img = cv2.cvtColor(undst, cv2.COLOR_BGR2RGB)
    pil_img = Image.fromarray(rgb_img)

    fov_h, fov_v = get_cropped_fov(new_camera_matrix, x, y, w, h)

    return pil_img, fov_h, fov_v

if __name__ == "__main__":
    img = Image.open("images/right_30.jpg")
    undst, fov_h, fov_v = undistort(img, "left")
    print(f"Field of View (cropped): {fov_h:.2f}° (horizontal), {fov_v:.2f}° (vertical)")
    print(f"Image size after cropping: {undst.size}")
    undst.show()