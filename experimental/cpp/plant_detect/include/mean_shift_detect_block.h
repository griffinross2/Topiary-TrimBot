#pragma once

#include <block.h>

class MeanShiftDetectBlock : public Block {
public:
    MeanShiftDetectBlock();
    MeanShiftDetectBlock(std::string id);
    void onUpdate() override;
};