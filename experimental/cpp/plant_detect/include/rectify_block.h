#pragma once

#include <block.h>

class RectifyBlock : public Block {
public:
    RectifyBlock();
    RectifyBlock(std::string id);
    void onUpdate() override;
};