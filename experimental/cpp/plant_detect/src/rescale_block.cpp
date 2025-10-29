#include "rescale_block.h"

#include "imgui.h"

RescaleBlock::RescaleBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Input Frame", cv::Mat()});
    m_outputs.push_back({"Scaled Frame", cv::Mat()});
}

RescaleBlock::RescaleBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Input Frame", cv::Mat()});
    m_outputs.push_back({"Scaled Frame", cv::Mat()});
}

void RescaleBlock::onUpdate() {
    Block::onUpdate();

    if (!m_inputs[0].newData) {
        return;
    }

    cv::Mat inFrame = std::get<cv::Mat>(m_inputs[0].data);

    if (inFrame.empty()) {
        return;
    }

    cv::Mat outFrame;

    cv::resize(inFrame, outFrame, cv::Size(m_width, m_height));

    m_outputs[0].data = outFrame;
    m_outputs[0].newData = true;
    m_inputs[0].newData = false;
}

void RescaleBlock::onRender() {
    ImGui::SetNextWindowSize(ImVec2(blockWidth + ioSize * 2, 150),
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

    ImGui::InputInt(std::format("Width##{}", m_id).c_str(), &m_width);
    ImGui::InputInt(std::format("Height##{}", m_id).c_str(), &m_height);
    ImGui::End();
}