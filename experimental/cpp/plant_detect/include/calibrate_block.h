#pragma once
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <block.h>

#include <array>

class CalibrateBlock : public Block {
public:
    CalibrateBlock();
    CalibrateBlock(std::string id);
    void onUpdate() override;
    void onRender() override;

private:
    void calibrate();

    int image_num = 1;
    std::vector<cv::Mat> m_frame;
    std::array<cv::Mat, 4> m_stereoMaps;
};