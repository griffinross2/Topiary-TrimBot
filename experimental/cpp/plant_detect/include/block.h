#pragma once

#include <vector>
#include <memory>
#include <array>
#include <variant>
#include <string>
#include <type_traits>

#include <opencv2/opencv.hpp>

#include "layer.h"

#define BLOCK_IO_TYPES                                                   \
    int, float, bool, std::string, std::vector<int>, std::vector<float>, \
        std::vector<bool>, std::vector<std::string>, cv::Mat

constexpr int blockWidth = 300;
constexpr int ioSize = 25;

typedef struct BlockIO {
    std::string name;
    std::variant<BLOCK_IO_TYPES> data;
} BlockIO;

typedef struct BlockConnection {
    std::string sourceId;
    std::string destId;
    int sourceIndex;
    int destIndex;
};

class Block : public Layer {
public:
    Block();
    Block(std::string id);
    ~Block() = default;
    const std::string& getId() { return m_id; }
    void setInput(int index, const std::variant<BLOCK_IO_TYPES>& value);
    std::variant<BLOCK_IO_TYPES> getOutput(int index);
    int getNumInputs() { return static_cast<int>(m_inputs.size()); }
    int getNumOutputs() { return static_cast<int>(m_outputs.size()); }
    void onUpdate();
    void onRender();
    void getPosition(int& x, int& y) {
        x = m_winX;
        y = m_winY;
    }

protected:
    void drawInputs();
    void drawOutputs();

    int m_winX = 0;
    int m_winY = 0;
    std::string m_id;
    std::vector<BlockIO> m_inputs;
    std::vector<BlockIO> m_outputs;
};

extern std::vector<std::shared_ptr<Block>> s_blocks;

template <typename T>
    requires std::is_base_of_v<Block, T>
void createBlock() {
    s_blocks.push_back(std::make_shared<T>());
}

template <typename T>
    requires std::is_base_of_v<Block, T>
void createBlock(std::string id) {
    s_blocks.push_back(std::make_shared<T>(id));
}

std::shared_ptr<Block> getBlockById(const std::string& blockId);

void startBlockConnection(const std::string& blockId,
                          int startIndex,
                          bool input);
void endBlockConnection(const std::string& blockId, int endIndex, bool input);

void updateBlocks();
void renderBlocks();
void renderBlockConnections();