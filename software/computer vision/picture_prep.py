from PIL import Image,ImageFilter

def picture_prep(img):
    img = img.rotate(180)

    w,h = img.size
    img = img.filter(ImageFilter.GaussianBlur(6))
    #img.show()
    img = img.resize([int(w/5),int(h/5)])
    #img.show()
    return(img, w, h)
