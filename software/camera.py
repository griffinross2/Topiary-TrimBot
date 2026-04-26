import cv2
import subprocess
import threading
import time

left_cam = None
right_cam = None

def init_cameras():
    global left_cam
    global right_cam

    cam0 = cv2.VideoCapture(0)
    cam1 = cv2.VideoCapture(4)

    # print(cam0.get(cv2.CAP_PROP_GAIN))
    # print(cam1.get(cv2.CAP_PROP_GAIN))

    # print(cam0.get(cv2.CAP_PROP_ISO_SPEED))
    # print(cam1.get(cv2.CAP_PROP_ISO_SPEED))

    # print(cam0.get(cv2.CAP_PROP_BRIGHTNESS))
    # print(cam1.get(cv2.CAP_PROP_BRIGHTNESS))

    # print(cam0.get(cv2.CAP_PROP_GAMMA))
    # print(cam1.get(cv2.CAP_PROP_GAMMA))

    # print(cam0.get(cv2.CAP_PROP_APERTURE))
    # print(cam1.get(cv2.CAP_PROP_APERTURE))

    def get_model(adapter_num):
        try:
            cmd = f"udevadm info --name=/dev/video{adapter_num:d} | grep ID_MODEL_ID"
            ser_out = subprocess.check_output(cmd, shell=True).decode('utf-8').strip()
            ser_num = ser_out.split('=')[1].split('\n')[0]
            # print(ser_num)
            return ser_num
        except Exception as e:
            print(e)
            return 0
        
    cam0_model = get_model(0)
    cam1_model = get_model(4)

    if cam1_model == "6366" and cam0_model == "0261":
        left_cam = cam0
        right_cam = cam1
    elif cam0_model == "6366" and cam1_model == "0261":
        left_cam = cam1
        right_cam = cam0
    else:
        print("Error reading serial numbers")
        return False

    left_cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
    left_cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
    right_cam.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
    right_cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

    left_cam.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1)
    right_cam.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1)

    left_cam.set(cv2.CAP_PROP_EXPOSURE, 3)
    right_cam.set(cv2.CAP_PROP_EXPOSURE, 17)

    left_cam.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    right_cam.set(cv2.CAP_PROP_BUFFERSIZE, 1)

    right_cam.set(cv2.CAP_PROP_SATURATION, 110)

    for i in range(30):
        left_cam.read()
        right_cam.read()

def save_image(angle, fname_left=None, fname_right=None):
    global left_cam
    global right_cam

    left_cam.read()
    right_cam.read()
    left_ret, left_frame = left_cam.read()
    right_ret, right_frame = right_cam.read()
    if left_ret and right_ret:
        # cv2.imshow("left_frame", left_frame)
        # cv2.imshow("right_frame", right_frame)
        left_frame = cv2.rotate(left_frame, cv2.ROTATE_180)
        right_frame = cv2.rotate(right_frame, cv2.ROTATE_180)
        if fname_left and fname_right:
            cv2.imwrite(fname_left, left_frame)
            cv2.imwrite(fname_right, right_frame)
        else:
            cv2.imwrite(f"images/left_{angle:d}.jpg", left_frame)
            cv2.imwrite(f"images/right_{angle:d}.jpg", right_frame)

def deinit_cameras():
    global left_cam
    global right_cam

    if left_cam:
        left_cam.release()
    if right_cam:
        right_cam.release()
