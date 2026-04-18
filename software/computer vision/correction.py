import numpy as np
from PIL import Image,ImageFilter
import cv2
import glob

def correction():
    #THIS FUNCTION IS CALLED ONCE, WRITES PARAMETERS TO FILE

    # Define the dimensions of checkerboard
    CHECKERBOARD = (6, 9)

    # stop the iteration when specified
    # accuracy, epsilon, is reached or
    # specified number of iterations are completed.
    criteria = (cv2.TERM_CRITERIA_EPS + 
                cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

    # Vector for 3D points
    threedpoints = []

    # Vector for 2D points
    twodpoints = []

    #  3D points real world coordinates
    objectp3d = np.zeros((1, CHECKERBOARD[0] 
                        * CHECKERBOARD[1], 
                        3), np.float32)
    objectp3d[0, :, :2] = np.mgrid[0:CHECKERBOARD[0],
                                0:CHECKERBOARD[1]].T.reshape(-1, 2)
    prev_img_shape = None

    # Extracting path of individual image stored
    # in a given directory. CHANGE WHEN IMPLEMENTED ON PI
    # jpg files alone
    images = glob.glob('/Users/duke1/OneDrive/Documents/GitHub/Topiary-TrimBot/software/computer vision/checkerboards/*.jpg')
    for filename in images:
        #print("searching: ", filename)
        image = cv2.imread(filename)
        grayColor = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

        # Find the chess board corners
        # If desired number of corners are
        # found in the image then ret = true
        ret, corners = cv2.findChessboardCorners(
                        grayColor, CHECKERBOARD, 
                        cv2.CALIB_CB_ADAPTIVE_THRESH 
                        + cv2.CALIB_CB_FAST_CHECK + 
                        cv2.CALIB_CB_NORMALIZE_IMAGE)

        # If desired number of corners can be detected then,
        # refine the pixel coordinates and display
        # them on the images of checker board
        if ret == True:
            #print("found")
            threedpoints.append(objectp3d)

            # Refining pixel coordinates
            # for given 2d points.
            corners2 = cv2.cornerSubPix(
                grayColor, corners, (11, 11), (-1, -1), criteria)

            twodpoints.append(corners2)

            # Draw and display the corners
            image = cv2.drawChessboardCorners(image, 
                                            CHECKERBOARD, 
                                            corners2, ret)

        # cv2.imshow('img', image)
        # cv2.waitKey(0)

    # cv2.destroyAllWindows()

    h, w = image.shape[:2]

    # Perform camera calibration by
    # passing the value of above found out 3D points (threedpoints)
    # and its corresponding pixel coordinates of the
    # detected corners (twodpoints)
    ret, matrix, distortion, r_vecs, t_vecs = cv2.calibrateCamera(
        threedpoints, twodpoints, grayColor.shape[::-1], None, None)

    newcameramtx, roi = cv2.getOptimalNewCameraMatrix(matrix, distortion, (w,h), 1, (w,h))

    file = cv2.FileStorage("parameters.yml", cv2.FileStorage_WRITE)
    file.write("matrix",matrix)
    file.write("distortion",distortion)
    file.write("newcameramtx",newcameramtx)
    file.write("x",roi[0])
    file.write("y",roi[1])
    file.write("w",roi[2])
    file.write("h",roi[3])

    file.release()

    return()

correction()