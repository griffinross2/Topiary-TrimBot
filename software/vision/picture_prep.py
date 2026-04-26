from PIL import Image,ImageFilter
from vision.undistort import *

def picture_prep(img,pos):
    #img = img.resize([1920,1080])
    img = undistort(img,pos) # uses OpenCV and calibration file to correct lens distortion

    w,h = img.size
    
    img = img.filter(ImageFilter.GaussianBlur(8)) # blends colors and reduces noise
    img = img.resize([int(w/8),int(h/8)]) # downsize to reduce runtime
    w,h = img.size
  
    return(img, w, h)
