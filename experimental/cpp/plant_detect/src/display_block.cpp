#include "display_block.h"

#include "imgui.h"
#include "glad/gl.h"

DisplayBlock::DisplayBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Frame", cv::Mat()});

    // Create texture for camera frame
    glGenTextures(1, &m_frameTexture);

    glBindTexture(GL_TEXTURE_2D, m_frameTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

DisplayBlock::DisplayBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Frame", cv::Mat()});

    // Create texture for camera frame
    glGenTextures(1, &m_frameTexture);

    glBindTexture(GL_TEXTURE_2D, m_frameTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void DisplayBlock::onRender() {
    cv::Mat& frame = std::get<cv::Mat>(m_inputs[0].data);
    GLsizei width = frame.cols;
    GLsizei height = frame.rows;

    ImGui::SetNextWindowSize(
        ImVec2(
            50 + ioSize * 2 + width,
            std::max(
                ioSize + (2 * std::max(m_inputs.size(), m_outputs.size()) - 1) *
                             ioSize,
                (unsigned long long)height)),
        ImGuiCond_Always);

    ImGui::Begin(m_id.c_str(), nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    int winX = ImGui::GetWindowPos().x;
    int winY = ImGui::GetWindowPos().y;

    m_winX = winX;
    m_winY = winY;

    drawInputs();

    drawOutputs();

    glBindTexture(GL_TEXTURE_2D, m_frameTexture);

    // Upload pixels into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
                 GL_UNSIGNED_BYTE, frame.data);

    ImGui::GetWindowDrawList()->AddImage(
        ImTextureRef(m_frameTexture),
        ImVec2(ImGui::GetWindowPos().x + 50 + ioSize * 2,
               ImGui::GetWindowPos().y),
        ImVec2(ImGui::GetWindowPos().x + 50 + ioSize * 2 + width,
               ImGui::GetWindowPos().y + height));

    ImGui::End();
}