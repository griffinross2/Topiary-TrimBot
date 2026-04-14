from PIL import Image,ImageFilter
from undistort import *

def picture_prep(img):
    img = undistort(img)
    img = img.rotate(180)
    w,h = img.size
    
    img = img.filter(ImageFilter.GaussianBlur(6))
    #img.show()
    img = img.resize([int(w/10),int(h/10)])
    w,h = img.size
    # img.show()
    return(img, w, h)
