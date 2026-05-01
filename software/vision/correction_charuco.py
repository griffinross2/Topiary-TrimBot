import cv2
import numpy as np
import glob

# Written by ChatGPT

# --------- SETTINGS ---------
# ChArUco board parameters (must match your printed board)
squares_x = 5
squares_y = 7
square_length = 0.029   # meters
marker_length = 0.0145   # meters

# ArUco dictionary
aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_100)

# Create ChArUco board
board = cv2.aruco.CharucoBoard(
    (squares_x, squares_y),
    square_length,
    marker_length,
    aruco_dict
)

# Image files
image_files = glob.glob("calibration_images/*.jpg")

# Storage
all_charuco_corners = []
all_charuco_ids = []
image_size = None

# --------- DETECTION ---------
for fname in image_files:
    img = cv2.imread(fname)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    if image_size is None:
        image_size = gray.shape[::-1]

    # Detect ArUco markers
    corners, ids, _ = cv2.aruco.detectMarkers(gray, aruco_dict)

    print("Detected marker IDs:", ids.flatten() if ids is not None else None)
    print("Board marker IDs:", board.getIds().flatten())

    if ids is not None and len(ids) > 0:
        # Refine detection
        cv2.aruco.refineDetectedMarkers(gray, board, corners, ids, rejectedCorners=None)

        # Interpolate ChArUco corners
        retval, charuco_corners, charuco_ids = cv2.aruco.interpolateCornersCharuco(
            corners, ids, gray, board
        )

        print(f"returned {retval}")

        # debug = cv2.aruco.drawDetectedMarkers(img.copy(), corners, ids)

        # if charuco_corners is not None:
        #     cv2.aruco.drawDetectedCornersCharuco(debug, charuco_corners, charuco_ids)

        # cv2.imshow("debug", debug)
        # cv2.waitKey(1000)

        print(charuco_corners, charuco_ids)

        if retval > 3:
            all_charuco_corners.append(charuco_corners)
            all_charuco_ids.append(charuco_ids)

# --------- CALIBRATION ---------
print("Calibrating...")

ret, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.aruco.calibrateCameraCharuco(
    charucoCorners=all_charuco_corners,
    charucoIds=all_charuco_ids,
    board=board,
    imageSize=image_size,
    cameraMatrix=None,
    distCoeffs=None
)

# --------- RESULTS ---------
print("\nCalibration successful:", ret)
print("\nCamera matrix:\n", camera_matrix)
print("\nDistortion coefficients:\n", dist_coeffs)

# Save results
np.savez("charuco_calibration.npz",
         camera_matrix=camera_matrix,
         dist_coeffs=dist_coeffs)

# --------- UNDISTORT EXAMPLE ---------
for image_file in image_files:
    img = cv2.imread(image_file)
    img_stem = image_file.split("\\")[-1].split(".")[0]


    h, w = img.shape[:2]
    new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(
        camera_matrix, dist_coeffs, (w, h), 1, (w, h)
    )

    undistorted = cv2.undistort(
        img, camera_matrix, dist_coeffs, None, new_camera_matrix
    )

    cv2.imwrite(f"undistorted/{img_stem}.jpg", undistorted)

    print(f"\nSaved undistorted/{img_stem}.jpg")