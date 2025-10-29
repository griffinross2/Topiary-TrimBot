#pragma once

#include <block.h>

class IntInputBlock : public Block {
public:
    IntInputBlock();
    IntInputBlock(std::string id);
    void onUpdate() override;
    void onRender() override;
};