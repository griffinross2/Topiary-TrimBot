#pragma once

#include <layer.h>

#include <memory>

#include <opencv2/opencv.hpp>

class CameraLayer : public Layer {
public:
    CameraLayer();
    ~CameraLayer() override;
    void onUpdate() override;
    void onRender() override;

private:
    int m_currentCameraIndex = 0;
    std::unique_ptr<cv::VideoCapture> m_camera;
    unsigned int m_frameTexture;
};