from PIL import Image,ImageFilter
from undistort import *

def picture_prep(img):
    img = img.resize([1920,1080])
    img = undistort(img)

    w,h = img.size
    
    img = img.filter(ImageFilter.GaussianBlur(7))
    #img.show()
    img = img.resize([int(w/10),int(h/10)]) #CHANGE WHEN ADJUSTING RESOLUTION
    w,h = img.size
    # img.show()
    return(img, w, h)
