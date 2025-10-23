#pragma once

#include <block.h>
#include <thread>

class CameraBlock : public Block {
public:
    CameraBlock();
    CameraBlock(std::string id);
    ~CameraBlock();
    void onUpdate() override;

private:
    void cameraThreadFunc();

    int m_currentCameraIndex = 0;
    std::unique_ptr<cv::VideoCapture> m_camera;
    std::thread m_cameraThread;
    cv::Mat m_internalFrame;
    std::mutex m_frameMutex;
};