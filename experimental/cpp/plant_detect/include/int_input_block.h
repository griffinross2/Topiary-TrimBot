#pragma once

#include <block.h>

class IntInputBlock : public Block {
public:
    IntInputBlock();
    IntInputBlock(std::string id);
    void onRender() override;
};