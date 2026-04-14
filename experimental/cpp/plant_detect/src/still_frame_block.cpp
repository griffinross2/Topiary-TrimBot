#include "still_frame_block.h"

#include "imgui.h"

StillFrameBlock::StillFrameBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Input Frame 1", cv::Mat()});
    m_inputs.push_back({"Input Frame 2", cv::Mat()});
    m_outputs.push_back({"Output Still 1", cv::Mat()});
    m_outputs.push_back({"Output Still 2", cv::Mat()});
}

StillFrameBlock::StillFrameBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Input Frame 1", cv::Mat()});
    m_inputs.push_back({"Input Frame 2", cv::Mat()});
    m_outputs.push_back({"Output Still 1", cv::Mat()});
    m_outputs.push_back({"Output Still 2", cv::Mat()});
}

void StillFrameBlock::onUpdate() {
    Block::onUpdate();

    if (!m_inputs[0].newData && !m_inputs[1].newData) {
        return;
    }

    cv::Mat inFrame = std::get<cv::Mat>(m_inputs[0].data);

    if (!inFrame.empty()) {
        m_internalFrame1 = inFrame.clone();
    }

    inFrame = std::get<cv::Mat>(m_inputs[1].data);

    if (!inFrame.empty()) {
        m_internalFrame2 = inFrame.clone();
    }

}

void StillFrameBlock::onRender() {
    ImGui::SetNextWindowSize(ImVec2(blockWidth + ioSize * 2, 140),
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

    ImGui::SetCursorPosY(100);

    if (ImGui::Button(std::format("Capture Still##{}", m_id).c_str())) {
        m_outputs[0].data = m_internalFrame1;
        m_outputs[0].newData = true;
        m_outputs[1].data = m_internalFrame2;
        m_outputs[1].newData = true;
        m_inputs[0].newData = false;
        m_inputs[1].newData = false;
    }

    ImGui::End();
}