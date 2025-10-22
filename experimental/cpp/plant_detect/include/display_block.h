#pragma once

#include <block.h>

class DisplayBlock : public Block {
public:
    DisplayBlock();
    DisplayBlock(std::string id);
    void onRender() override;

private:
    unsigned int m_frameTexture;
};