#include "mean_shift_detect_block.h"

#include "imgui.h"

void MeanShiftDetectBlock::insertMean(std::vector<std::pair<int, int>>& means, int mean) {
    for (auto& hue : means) {
        if (hue.first == mean) {
            hue.second++;
            return;
        }
    }

    means.push_back(std::make_pair(mean, 1));
}

std::vector<int> MeanShiftDetectBlock::pointsInDist(int center, int dist) {
    std::vector<int> points;
    int min = std::max(0, center - dist);
    int max = std::min(179, center + dist);
    for (int i = min; i <= max; i++) {
        points.push_back(i);
    }

    return points;
}

std::vector<int> MeanShiftDetectBlock::filterPoints(
    const std::vector<int>& hues,
                  const std::vector<int>& points) {
    std::vector<int> filteredPoints;
    for (int p : points) {
        if (hues[p] > 0) {
            filteredPoints.push_back(p);
        }
    }
    return filteredPoints;
}

int MeanShiftDetectBlock::findMean(const std::vector<int>& hues,
              int hueStart) {
    int hueMean = hueStart;

    std::vector<int> points = pointsInDist(hueMean, m_clusterRadius);

    for (int it = 0; it < m_maxIterations; it++) {
        std::vector<int> filteredPoints = filterPoints(hues, points);

        int sum = 0;
        for (int p : filteredPoints) {
            sum += p;
        }
        int newHueMean = sum / filteredPoints.size();

        if (newHueMean == hueMean) {
            break;
        }

        int deltaMean = newHueMean - hueMean;
        hueMean = newHueMean;
        for (int& p : points) {
            p += deltaMean;
            p = std::max(0, std::min(179, p));
        }
    }

    return hueMean;
}

MeanShiftDetectBlock::MeanShiftDetectBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Input Frame", cv::Mat()});
    m_outputs.push_back({"Output Mask", cv::Mat()});
}

MeanShiftDetectBlock::MeanShiftDetectBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Input Frame", cv::Mat()});
    m_outputs.push_back({"Output Mask", cv::Mat()});
}

void MeanShiftDetectBlock::onUpdate() {
    Block::onUpdate();

    if (!m_inputs[0].newData) {
        return;
    }

    cv::Mat inFrame = std::get<cv::Mat>(m_inputs[0].data);

    if (inFrame.empty()) {
        return;
    }

    cv::Mat hsvFrame;
    cv::cvtColor(inFrame, hsvFrame, cv::COLOR_BGR2HSV);

    cv::Mat outFrame(inFrame.size(), CV_8UC3, cv::Scalar(0));

    // Count hues
    std::vector<int> hueCounts;
    hueCounts.resize(180, 0);
    for (int r = 0; r < inFrame.rows; r++) {
        for (int c = 0; c < inFrame.cols; c++) {
            cv::Vec3b hsvPixel = hsvFrame.at<cv::Vec3b>(r, c);
            int hue = hsvPixel[0];
            float saturation = hsvPixel[1] / 255.0f;
            float value = hsvPixel[2] / 255.0f;
            if (saturation >= m_minSaturation && value >= m_minValue &&
                value <= m_maxValue) {
                hueCounts[hue]++;
            }
        }
    }

    // Filter hues that aren't prominent
    for (int i = 0; i < 180; i++) {
        if (hueCounts[i] < m_minPixelsOfColor) {
            hueCounts[i] = 0;
        }
    }

    // Get all means
    std::vector<std::pair<int, int>> means;
    for (int i = 0; i < 180; i++) {
        if (hueCounts[i] > 0) {
            int meanHue = findMean(hueCounts, i);
            insertMean(means, meanHue);
        }
    }

    // Select means near MAGIC_HUE
    std::vector<int> selectedMeans;
    for (auto& mean : means) {
        if (std::abs(mean.first - m_magicHue) <= m_clusterSelectionDistance) {
            selectedMeans.push_back(mean.first);
        }
    }

    // Go through image and create mask
    for (int r = 0; r < inFrame.rows; r++) {
        for (int c = 0; c < inFrame.cols; c++) {
            cv::Vec3b hsvPixel = hsvFrame.at<cv::Vec3b>(r, c);
            int hue = hsvPixel[0];
            float saturation = hsvPixel[1] / 255.0f;
            float value = hsvPixel[2] / 255.0f;
            
            if (hueCounts[hue] == 0 ||
                saturation < m_minSaturation ||
                value < m_minValue ||
                value > m_maxValue) {
                continue;
            }

            // See if this pixel ends up in a selected cluster
            int meanHue = findMean(hueCounts, hue);
            for (int selMean : selectedMeans) {
                if (selMean == meanHue) {
                    outFrame.at<cv::Vec3b>(r, c) = cv::Vec3b(0, 255, 0);
                    break;
                }
            }
        }
    }

    m_outputs[0].data = outFrame;
    m_outputs[0].newData = true;
    m_inputs[0].newData = false;
}

void MeanShiftDetectBlock::onRender() {
    ImGui::SetNextWindowSize(ImVec2(blockWidth + ioSize * 2, 300),
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

    ImGui::SliderInt(std::format("Magic Hue##{}", m_id).c_str(), &m_magicHue, 0,
                     179);
    ImGui::InputInt(std::format("Cluster Radius##{}", m_id).c_str(),
                    &m_clusterRadius);
    ImGui::InputInt(std::format("Max Iterations##{}", m_id).c_str(),
                    &m_maxIterations);
    ImGui::InputInt(std::format("Min Pixels of Color##{}", m_id).c_str(),
                    &m_minPixelsOfColor);
    ImGui::SliderFloat(std::format("Min Saturation##{}", m_id).c_str(),
                       &m_minSaturation, 0.0f, 1.0f);
    ImGui::SliderFloat(std::format("Min Value##{}", m_id).c_str(), &m_minValue, 0.0f, 1.0f);
    ImGui::SliderFloat(std::format("Max Value##{}", m_id).c_str(), &m_maxValue,
                       0.0f, 1.0f);
    ImGui::InputInt(std::format("Cluster Selection Distance##{}", m_id).c_str(),
                    &m_clusterSelectionDistance);

    ImGui::End();
}