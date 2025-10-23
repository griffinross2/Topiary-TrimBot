#pragma once

#include "layer.h"

#include "imgui.h"

#include <array>

constexpr ImGuiKey DEBUG_OVERLAY_TOGGLE = ImGuiKey_F3;
constexpr int DEBUG_FRAME_TIME_HISTORY_SIZE = 200;
constexpr float DEBUG_GRAPH_MAX_TIME = 0.1f;  // seconds
constexpr float DEBUG_GRAPH_WIDTH = 200.0f;   // pixels
constexpr float DEBUG_GRAPH_HEIGHT = 75.0f;   // pixels

class DebugLayer : public Layer {
public:
    DebugLayer();
    ~DebugLayer();
    void onUpdate() override;
    void onRender() override;

private:
#ifdef NDEBUG
    bool m_visible = false;
#else
    bool m_visible = true;
#endif
    std::array<float, DEBUG_FRAME_TIME_HISTORY_SIZE> m_frameTimes;
    int m_frameTimeSize = 0;
};
