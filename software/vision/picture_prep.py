from PIL import Image,ImageFilter,ImageOps
from vision.undistort import *

def picture_prep(img,pos):
    #img = img.resize([1920,1080])
    img, fov_h, fov_v = undistort(img,pos) # uses OpenCV and calibration file to correct lens distortion
    org_w,org_h = img.size
    img = ImageOps.expand(img, border=50, fill='white') # Padding in case edges are cut off
    w,h = img.size
    # Then need to adjust FOVs to account


    
    img = img.filter(ImageFilter.GaussianBlur(8)) # blends colors and reduces noise
    img = img.resize([int(w/8),int(h/8)]) # downsize to reduce runtime
    w,h = img.size
    org_w, org_h = org_w//8, org_h//8
  
    return img, w, h, org_w, org_h, fov_h, fov_v
