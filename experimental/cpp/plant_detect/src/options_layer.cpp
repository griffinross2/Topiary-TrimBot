#include "options_layer.h"

#include "options.h"

#include "imgui.h"

OptionsLayer::OptionsLayer() : Layer() {}

OptionsLayer::~OptionsLayer() = default;

void OptionsLayer::onUpdate() {
    Options::setCamera(m_cameraIndexInput);
}

void OptionsLayer::onRender() {
    ImGui::Begin("Options");

    ImGui::InputInt("Camera Index", &m_cameraIndexInput);

    ImGui::End();
}