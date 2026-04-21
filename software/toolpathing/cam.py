import cv2

cam = cv2.VideoCapture(2)
# cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
# cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
# cam.set(cv2.CAP_PROP_AUTO_WB, 0.0)
# cam.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0.25)
# cam.set(cv2.CAP_PROP_EXPOSURE, -16.0)
# cam.set(cv2.CAP_PROP_AUTO_WB, 0.0)
# cam.set(cv2.CAP_PROP_WB_TEMPERATURE, 4500)
# cam.set(cv2.CAP_PROP_GAIN, 0.0)

# angle = 0

for dummy in range(10):
    ret, frame = cam.read()

ret, frame = cam.read()
if ret:
    cv2.imshow("frame", frame)
    cv2.waitKey(0)
    cv2.imwrite("test1.png", frame)

# for angle in range(0, 360, 20):
#     ret, frame = cam.read()
#     if ret:
#         cv2.imshow("frame", frame)
#         cv2.waitKey(0)
#         cv2.imwrite(f"right_middle_{angle}.jpg", frame)