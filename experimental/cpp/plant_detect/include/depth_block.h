#pragma once

#include <block.h>

#include <utility>

class DepthBlock : public Block {
public:
    DepthBlock();
    DepthBlock(std::string id);
    void onUpdate() override;
    void onRender() override;

private:
    int m_numDisparities = 16;
    int m_blockSize = 5;
    int m_preFilterType = 1;
    int m_preFilterSize = 5;
    int m_preFilterCap = 31;
    int m_minDisparity = 0;
    int m_textureThreshold = 10;
    int m_uniquenessRatio = 15;
    int m_speckleRange = 0;
    int m_speckleWindowSize = 0;
    int m_disp12MaxDiff = 0;
    int m_dispType = CV_16S;
};