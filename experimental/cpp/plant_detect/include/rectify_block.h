#pragma once

#include <block.h>

class GrayscaleBlock : public Block {
public:
    GrayscaleBlock();
    GrayscaleBlock(std::string id);
    void onUpdate() override;
};