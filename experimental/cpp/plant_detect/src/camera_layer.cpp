#include "camera_layer.h"
#include "options.h"

#include <iostream>

#include "imgui.h"
#include "glad/gl.h"

CameraLayer::CameraLayer() : Layer() {
    // Get camera and set resolution
    m_camera = std::make_unique<cv::VideoCapture>(m_currentCameraIndex);
    double width = m_camera->get(cv::CAP_PROP_FRAME_WIDTH);
    double height = m_camera->get(cv::CAP_PROP_FRAME_HEIGHT);
    Options::setResolution(static_cast<int>(width), static_cast<int>(height));

    // Create texture for camera frame
    glGenTextures(1, &m_frameTexture);

    glBindTexture(GL_TEXTURE_2D, m_frameTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

CameraLayer::~CameraLayer() {
    glDeleteTextures(1, &m_frameTexture);
}

void CameraLayer::onUpdate() {
    if (Options::getCamera() != m_currentCameraIndex) {
        m_currentCameraIndex = Options::getCamera();
        m_camera->open(m_currentCameraIndex);
        double width = m_camera->get(cv::CAP_PROP_FRAME_WIDTH);
        double height = m_camera->get(cv::CAP_PROP_FRAME_HEIGHT);
        Options::setResolution(static_cast<int>(width),
                               static_cast<int>(height));
    }
}

void CameraLayer::onRender() {
    // Set window size
    if (m_camera->isOpened()) {
        int width, height;
        Options::getResolution(width, height);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    } else {
        ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);
    }

    ImGui::Begin("Camera");

    if (!m_camera->isOpened()) {
        ImGui::Text("Error: Could not open camera.");
        ImGui::End();
        return;
    }

    cv::Mat frame;
    m_camera->read(frame);

    if (frame.empty()) {
        return;
    }

    GLsizei width = frame.cols;
    GLsizei height = frame.rows;

    glBindTexture(GL_TEXTURE_2D, m_frameTexture);

    std::cout << frame.type() << std::endl;

    // Upload pixels into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
                 GL_UNSIGNED_BYTE, frame.data);

    ImGui::GetWindowDrawList()->AddImage(
        ImTextureRef(m_frameTexture),
        ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y),
        ImVec2(ImGui::GetWindowPos().x + width,
               ImGui::GetWindowPos().y + height));

    ImGui::End();
}