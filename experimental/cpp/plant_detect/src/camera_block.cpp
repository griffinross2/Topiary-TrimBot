#include "camera_block.h"

#include "imgui.h"

CameraBlock::CameraBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Camera Index", int{0}});
    m_outputs.push_back({"Frame", cv::Mat()});

    // Get camera
    m_camera = std::make_unique<cv::VideoCapture>(m_currentCameraIndex);
}

CameraBlock::CameraBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Camera Index", int{0}});
    m_outputs.push_back({"Frame", cv::Mat()});
}

void CameraBlock::onUpdate() {
    Block::onUpdate();

    if (std::get<int>(m_inputs[0].data) != m_currentCameraIndex) {
        m_currentCameraIndex = std::get<int>(m_inputs[0].data);
        m_camera->open(m_currentCameraIndex);
    }

    if (!m_camera->isOpened()) {
        return;
    }

    cv::Mat frame;
    m_camera->read(frame);

    if (frame.empty()) {
        return;
    }

    m_outputs[0].data = frame;
}