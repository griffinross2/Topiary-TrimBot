#pragma once

#include <block.h>

#include <utility>

class MeanShiftDetectBlock : public Block {
public:
    MeanShiftDetectBlock();
    MeanShiftDetectBlock(std::string id);
    void onUpdate() override;
    void onRender() override;

private:
    void insertMean(std::vector<std::pair<int, int>>& means, int mean);
    std::vector<int> pointsInDist(int center, int dist);
    std::vector<int> filterPoints(const std::vector<int>& hues, const std::vector<int>& points);
    int findMean(const std::vector<int>& hues, int hueStart);

    int m_magicHue = 42;       // Hue for green plant
    int m_clusterRadius = 10;  // Radius for mean shift clustering
    int m_maxIterations = 20;  // Max iterations for search
    int m_minPixelsOfColor =
        500;  // Minimum pixels to cluster a color
    float m_minSaturation =
        0.15f;                         // Minimum saturation to consider a pixel
    float m_minValue = 0.1f;  // Minimum value to consider a pixel
    float m_maxValue = 0.93f;  // Maximum value to consider a pixel
    int m_clusterSelectionDistance =
        18;  // Max distance from the magic hue to select a cluster
};