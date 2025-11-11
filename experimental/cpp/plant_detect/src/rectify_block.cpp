#include "grayscale_block.h"

#include "imgui.h"

GrayscaleBlock::GrayscaleBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Color Frame", cv::Mat()});
    m_outputs.push_back({"Gray Frame", cv::Mat()});
}

GrayscaleBlock::GrayscaleBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Color Frame", cv::Mat()});
    m_outputs.push_back({"Gray Frame", cv::Mat()});
}

void GrayscaleBlock::onUpdate() {
    Block::onUpdate();

    if (!m_inputs[0].newData) {
        return;
    }

    cv::Mat inFrame = std::get<cv::Mat>(m_inputs[0].data);

    if (inFrame.empty()) {
        return;
    }

    cv::Mat gray1;
    cv::Mat outFrame;

    cv::cvtColor(inFrame, gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(gray1, outFrame, cv::COLOR_GRAY2BGR);

    m_outputs[0].data = outFrame;
    m_outputs[0].newData = true;
    m_inputs[0].newData = false;
}