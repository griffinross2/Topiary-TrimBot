#pragma once

#include <block.h>
#include <thread>

#include "libcamera/libcamera.h"

class CameraBlock : public Block {
public:
    CameraBlock();
    CameraBlock(std::string id);
    ~CameraBlock();
    void onUpdate() override;

private:
    void cameraThreadFunc();

    int m_currentCameraIndex = 0;
#ifndef __linux__
    std::unique_ptr<cv::VideoCapture> m_camera;
#else
    std::unique_ptr<libcamera::Camera> m_camera;
#endif
    std::thread m_cameraThread;
    cv::Mat m_internalFrame;
    bool m_newFrame = false;
    std::mutex m_frameMutex;
};
