#pragma once

#include <block.h>

class CameraBlock : public Block {
public:
    CameraBlock();
    CameraBlock(std::string id);
    void onUpdate() override;

private:
    int m_currentCameraIndex = 0;
    std::unique_ptr<cv::VideoCapture> m_camera;
};