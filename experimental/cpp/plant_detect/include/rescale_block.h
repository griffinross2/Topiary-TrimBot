#pragma once

#include <block.h>

class RescaleBlock : public Block {
public:
    RescaleBlock();
    RescaleBlock(std::string id);
    void onRender() override;
    void onUpdate() override;

private:
    int m_width = 1280;
    int m_height = 720;
};