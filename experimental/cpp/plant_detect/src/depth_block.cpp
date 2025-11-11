#include "depth_block.h"

#include "imgui.h"

cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create();

DepthBlock::DepthBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Left Frame", cv::Mat()});
    m_inputs.push_back({"Right Frame", cv::Mat()});
    m_outputs.push_back({"Depth Map", cv::Mat()});
}

DepthBlock::DepthBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Left Frame", cv::Mat()});
    m_inputs.push_back({"Right Frame", cv::Mat()});
    m_outputs.push_back({"Depth Map", cv::Mat()});
}

void DepthBlock::onUpdate() {
    Block::onUpdate();

    // Only proceed when at least one input has new data
    if (!m_inputs[0].newData && !m_inputs[1].newData) {
        return;
    }

    cv::Mat leftFrame = std::get<cv::Mat>(m_inputs[0].data);
    cv::Mat rightFrame = std::get<cv::Mat>(m_inputs[1].data);

    // Make sure we have both frames before computing disparity
    if (leftFrame.empty() || rightFrame.empty()) {
        // Clear new-data flags to avoid repeatedly trying with empty data
        m_inputs[0].newData = false;
        m_inputs[1].newData = false;
        return;
    }

    cv::Mat leftGray;
    cv::Mat rightGray;
    cv::Mat outFrame;

    /*cv::Mat Left_Stereo_Map1, Left_Stereo_Map2;
    cv::Mat Right_Stereo_Map1, Right_Stereo_Map2;*/

    cv::Mat disp, disparity;

    cv::cvtColor(leftFrame, leftGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(rightFrame, rightGray, cv::COLOR_BGR2GRAY);

    // apply UI params to the StereoBM instance (before stereo->compute)
    int numDisp =
        std::max(1, m_numDisparities) * 16;  // UI likely uses "n * 16"
    stereo->setNumDisparities(numDisp);
    stereo->setBlockSize((m_blockSize % 2 == 1)
                             ? m_blockSize
                             : (m_blockSize | 1));  // ensure odd
    stereo->setPreFilterType(m_preFilterType);
    stereo->setPreFilterSize(m_preFilterSize);
    stereo->setPreFilterCap(m_preFilterCap);
    stereo->setTextureThreshold(m_textureThreshold);
    stereo->setUniquenessRatio(m_uniquenessRatio);
    stereo->setSpeckleRange(m_speckleRange);
    stereo->setSpeckleWindowSize(m_speckleWindowSize);
    stereo->setDisp12MaxDiff(m_disp12MaxDiff);

    // NEED RECTIFICATION MAPS HERE (if you have calibrated camera pair)

    stereo->compute(leftGray, rightGray, disp);

    // Converting disparity values to CV_32F from CV_16S
    disp.convertTo(disparity, CV_32F, 1.0);
    // Scaling down the disparity values and normalizing them
    disparity =
        (disparity / 16.0f - (float)m_minDisparity) / ((float)m_numDisparities);

    disp.convertTo(disparity, CV_8U, 1.0);

    cv::cvtColor(disparity, outFrame, cv::COLOR_GRAY2BGR);

    m_outputs[0].data = outFrame;
    m_outputs[0].newData = true;

    // Clear both input new-data flags (we consumed the frames)
    m_inputs[0].newData = false;
    m_inputs[1].newData = false;
}

void DepthBlock::onRender() {
    // Creating trackbars to dynamically update the StereoBM parameters
    ImGui::SetNextWindowSize(ImVec2(blockWidth + ioSize * 2, 450),
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

    // Sets the range of disparity values to be searched. The overall range is
    // from minimum to minimum disparity value + several disparities. Increasing
    // the number of disparities increases the accuracy of the disparity map.
    ImGui::SliderInt(std::format("numDisparities##{}", m_id).c_str(),
                     &m_numDisparities, 1, 18);

    // Size of the sliding window used for block matching to find corresponding
    // pixels in a rectified stereo image pair. A higher value indicates a
    // larger window size. The following GIF indicates that increasing this
    // parameter results in more smooth disparity maps.
    ImGui::SliderInt(std::format("blockSize##{}", m_id).c_str(), &m_blockSize,
                     1, 50);

    // Parameter to decide the type of pre-filtering to be applied to the images
    // before passing to the block matching algorithm. This step enhances the
    // texture information and improves the results of the block-matching
    // algorithm. The filter type can be CV_STEREO_BM_XSOBEL or
    // CV_STEREO_BM_NORMALIZED_RESPONSE.
    ImGui::SliderInt(std::format("preFilterType##{}", m_id).c_str(),
                     &m_preFilterType, 0, 1);

    // Window size of the filter used in the pre-filtering stage
    ImGui::InputInt(std::format("preFilterSize 5 to 255 odd##{}", m_id).c_str(),
                    &m_preFilterSize);

    // Limits the filtered output to a specific value.
    ImGui::SliderInt(std::format("preFilterCap##{}", m_id).c_str(),
                     &m_preFilterCap, 1, 62);

    // Filters out areas that do not have enough texture information for
    // reliable matching.
    ImGui::SliderInt(std::format("textureThreshold##{}", m_id).c_str(),
                     &m_textureThreshold, 1, 100);

    // Another post-filtering step. The pixel is filtered out if the best
    // matching disparity is not sufficiently better than every other disparity
    // in the search range. The following GIF depicts that increasing the
    // uniqueness ratio increases the number of pixels that are filtered out.
    ImGui::SliderInt(std::format("uniquenessRatio##{}", m_id).c_str(),
                     &m_uniquenessRatio, 1, 100);

    // Speckles are produced near the boundaries of the objects, where the
    // matching window catches the foreground on one side and the background on
    // the other. To get rid of these artifacts, we apply speckle filter which
    // has two parameters. The speckle range defines how close the disparity
    // values should be to be considered as part of the same blob. The speckle
    // window size is the number of pixels below which a disparity blob is
    // dismissed as “speckle”.
    ImGui::SliderInt(std::format("speckleRange##{}", m_id).c_str(),
                     &m_speckleRange, 0, 100);
    ImGui::SliderInt(std::format("speckleWindowSize##{}", m_id).c_str(),
                     &m_speckleWindowSize, 0, 25);

    // Pixels are matched both ways, from the left image to the right image and
    // from the right image to left image. disp12MaxDiff defines the maximum
    // allowable difference between the original left pixel and the back-matched
    // pixel.
    ImGui::SliderInt(std::format("disp12MaxDiff##{}", m_id).c_str(),
                     &m_disp12MaxDiff, 0, 25);

    // The minimum value of the disparity to be searched. In most scenarios it
    // is set to zero. It can also be set to negative value depending on the
    // stereo camera setup.
    ImGui::SliderInt(std::format("minDisparity##{}", m_id).c_str(),
                     &m_minDisparity, 0, 25);

    ImGui::End();
}