#include "camera_block.h"

#include "imgui.h"

CameraBlock::CameraBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Camera Index", int{0}, true});
    m_outputs.push_back({"Frame", cv::Mat(), true});

    // Get camera
    m_camera = std::make_unique<cv::VideoCapture>(m_currentCameraIndex);

    m_cameraThread = std::thread([this]() { this->cameraThreadFunc(); });
}

CameraBlock::CameraBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Camera Index", int{0}, true});
    m_outputs.push_back({"Frame", cv::Mat(), true});

    // Get camera
    m_camera = std::make_unique<cv::VideoCapture>(m_currentCameraIndex);

    m_cameraThread = std::thread([this]() { this->cameraThreadFunc(); });
}

CameraBlock::~CameraBlock() {
    if (m_cameraThread.joinable()) {
        m_cameraThread.join();
    }
}

void CameraBlock::onUpdate() {
    Block::onUpdate();

    // Input and output frame
    m_frameMutex.lock();
    if (m_newFrame) {
        m_outputs[0].data = m_internalFrame;
        m_outputs[0].newData = true;
        m_newFrame = false;
    }
    m_frameMutex.unlock();
}

void CameraBlock::cameraThreadFunc() {
    while (true) {
        if (!m_camera->isOpened()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (std::get<int>(m_inputs[0].data) != m_currentCameraIndex) {
            m_currentCameraIndex = std::get<int>(m_inputs[0].data);
            m_camera->open(m_currentCameraIndex);
        }

        if (!m_camera->isOpened()) {
            continue;
        }

        cv::Mat frame;
        m_camera->read(frame);

        if (frame.empty()) {
            continue;
        }

        m_frameMutex.lock();
        m_internalFrame = frame;
        m_newFrame = true;
        m_frameMutex.unlock();
    }
}