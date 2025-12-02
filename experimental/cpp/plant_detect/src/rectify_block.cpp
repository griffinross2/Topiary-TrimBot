#include "rectify_block.h"

#include "imgui.h"

RectifyBlock::RectifyBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Unrectified Frame", cv::Mat()});
    m_outputs.push_back({"Rectified Frame", cv::Mat()});
}

RectifyBlock::RectifyBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Unrectified Frame", cv::Mat()});
    m_outputs.push_back({"Rectified Frame", cv::Mat()});
}

void RectifyBlock::onUpdate() {
    Block::onUpdate();

    if (!m_inputs[0].newData) {
        return;
    }

    cv::Mat inFrame = std::get<cv::Mat>(m_inputs[0].data);

    if (inFrame.empty()) {
        return;
    }

    cv::Mat outFrame;

    cv::cvtColor(inFrame, outFrame, cv::COLOR_BGR2GRAY);

    m_outputs[0].data = outFrame;
    m_outputs[0].newData = true;
    m_inputs[0].newData = false;
}