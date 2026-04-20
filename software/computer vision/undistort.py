import numpy as np
from PIL import Image,ImageFilter
import cv2
import glob

def undistort(img):
    # undistort
    cv_img = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)

    file = cv2.FileStorage("parameters.yml", cv2.FileStorage_READ)
    matrix = file.getNode("matrix").mat()
    distortion = file.getNode("distortion").mat()
    newcameramtx = file.getNode("newcameramtx").mat()
    x = int(file.getNode("x").real())
    y = int(file.getNode("y").real())
    w = int(file.getNode("w").real())
    h = int(file.getNode("h").real())
    
    undst = cv2.undistort(cv_img, matrix, distortion, None, newcameramtx)


    # cv2.imshow('original', cv_img)
    # cv2.imshow('undistorted', undst)
    # cv2.waitKey(0)

    undst = undst[y:y+h, x:x+w]
    rgb_img = cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB)
    pil_img = Image.fromarray(rgb_img)
    return(pil_img)