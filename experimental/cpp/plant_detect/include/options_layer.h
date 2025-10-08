#pragma once

#include <layer.h>

class OptionsLayer : public Layer {
public:
    OptionsLayer();
    ~OptionsLayer() override;
    void onUpdate() override;
    void onRender() override;

private:
    int m_cameraIndexInput = 0;
};