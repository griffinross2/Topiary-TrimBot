#include "calibrate_block.h"

#include <opencv2/core/types_c.h>
#include <opencv2/calib3d.hpp>
#include "imgui.h"

CalibrateBlock::CalibrateBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Left Frame", cv::Mat()});
    m_inputs.push_back({"Right Frame", cv::Mat()});
    m_outputs.push_back({"Left Frame", cv::Mat()});
    m_outputs.push_back({"Right Frame", cv::Mat()});
    m_outputs.push_back({"Calibration Frames", cv::Mat()});
    calibrate();
}

CalibrateBlock::CalibrateBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Left Frame", cv::Mat()});
    m_inputs.push_back({"Right Frame", cv::Mat()});
    m_outputs.push_back({"Left Frame", cv::Mat()});
    m_outputs.push_back({"Right Frame", cv::Mat()});
    m_outputs.push_back({"Calibration Frames", cv::Mat()});
    calibrate();
}

void CalibrateBlock::onUpdate() {
    Block::onUpdate();

    cv::Mat leftFrame = std::get<cv::Mat>(m_inputs[0].data);
    cv::Mat rightFrame = std::get<cv::Mat>(m_inputs[1].data);

    // Make sure we have both frames
    if (leftFrame.empty() || rightFrame.empty()) {
        // Clear new-data flags to avoid repeatedly trying with empty data
        m_inputs[0].newData = false;
        m_inputs[1].newData = false;
        return;
    }

    cv::Mat leftGray;
    cv::Mat rightGray;
    cv::Mat outFrame;

    cv::cvtColor(leftFrame, leftGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(rightFrame, rightGray, cv::COLOR_BGR2GRAY);

    cv::Mat leftNice, rightNice, leftOut, rightOut;
    cv::remap(leftGray, leftNice, m_stereoMaps[0], m_stereoMaps[1],
              cv::INTER_LANCZOS4, cv::BORDER_CONSTANT, 0);
    cv::remap(rightGray, rightNice, m_stereoMaps[2], m_stereoMaps[3],
              cv::INTER_LANCZOS4, cv::BORDER_CONSTANT, 0);

    m_inputs[0].newData = false;
    m_inputs[1].newData = false;

    cv::cvtColor(leftNice, leftOut, cv::COLOR_GRAY2BGR);
    cv::cvtColor(rightNice, rightOut, cv::COLOR_GRAY2BGR);

    m_outputs[0].data = leftOut;
    m_outputs[1].data = rightOut;
    m_outputs[0].newData = true;
    m_outputs[1].newData = true;
    m_outputs[2].newData = true;
}

void CalibrateBlock::calibrate() {
    int CHECKERBOARD[2]{6, 9};
    // Creating vector to store vectors of 3D points for each checkerboard image
    std::vector<std::vector<cv::Point3f> > objpoints;

    // Creating vector to store vectors of 2D points for each checkerboard image
    std::vector<std::vector<cv::Point2f> > imgpoints;

    // Defining the world coordinates for 3D points
    std::vector<cv::Point3f> objp;
    for (int i{0}; i < CHECKERBOARD[1]; i++) {
        for (int j{0}; j < CHECKERBOARD[0]; j++)
            objp.push_back(cv::Point3f(j, i, 0));
    }

    // Extracting path of individual image stored in a given directory
    std::vector<cv::String> images;

    // Path of the folder containing checkerboard images
    std::string path = "./images/*.jpg";

    cv::glob(path, images);
    std::vector<cv::Mat> gray(images.size());
    m_frame.resize(images.size());
    // vector to store the pixel coordinates of detected checker board corners
    std::vector<cv::Point2f> corner_pts;
    bool success;

    // Looping over all the images in the directory
    for (int i{0}; i < images.size(); i++) {
        m_frame[i] = cv::imread(images[i]);
        cv::cvtColor(m_frame[i], gray[i], cv::COLOR_BGR2GRAY);

        success = cv::findChessboardCorners(
            gray[i], cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts,
            cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK |
                cv::CALIB_CB_NORMALIZE_IMAGE);

        if (success) {
            cv::TermCriteria criteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30,
                                      0.001);

            // refining pixel coordinates for given 2d points.
            cv::cornerSubPix(gray[i], corner_pts, cv::Size(11, 11),
                             cv::Size(-1, -1), criteria);

            // Displaying the detected corner points on the checker board
            cv::drawChessboardCorners(
                m_frame[i], cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]),
                corner_pts, success);

            objpoints.push_back(objp);
            imgpoints.push_back(corner_pts);
        }
    }

    cv::Mat cameraMatrix, distCoeffs, R, T;

    cv::Mat Rot, Trns, Emat, Fmat;
    int flag = 0;
    flag |= cv::CALIB_SAME_FOCAL_LENGTH;
    cv::stereoCalibrate(
        objpoints, imgpoints, imgpoints, cameraMatrix, distCoeffs, cameraMatrix,
        distCoeffs, gray[0].size(), Rot, Trns, Emat, Fmat, flag,
        cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 30,
                         1e-6));

    // Once we know the transformation between the two cameras we can perform
    // stereo rectification
    cv::Mat rect_l, rect_r, proj_mat_l, proj_mat_r, Q;
    cv::stereoRectify(cameraMatrix, distCoeffs, cameraMatrix, distCoeffs,
                      gray[0].size(), Rot, Trns, rect_l, rect_r, proj_mat_l,
                      proj_mat_r, Q, 1);

    cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, rect_l, proj_mat_l,
                                gray[0].size(), CV_16SC2, m_stereoMaps[0],
                                m_stereoMaps[1]);

    cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, rect_r, proj_mat_r,
                                gray[0].size(), CV_16SC2, m_stereoMaps[2],
                                m_stereoMaps[3]);

    // output checkerboard detected corner frames, output stereo maps
    m_outputs[2].data = m_frame[image_num - 1];
}

void CalibrateBlock::onRender() {
    // Creating trackbars to dynamically update the StereoBM parameters
    ImGui::SetNextWindowSize(ImVec2(blockWidth + ioSize * 2, 160),
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

    ImGui::SetCursorPosY(120);

    if (ImGui::SliderInt(std::format("Image##{}", m_id).c_str(), &image_num, 1,
                         20)) {
        m_outputs[2].data = m_frame[image_num - 1];
    }

    ImGui::End();
}