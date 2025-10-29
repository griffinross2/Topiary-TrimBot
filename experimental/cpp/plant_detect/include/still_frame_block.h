#pragma once

#include <block.h>

class StillFrameBlock : public Block {
public:
    StillFrameBlock();
    StillFrameBlock(std::string id);
    void onRender() override;
    void onUpdate() override;

    private:
    cv::Mat m_internalFrame;
};