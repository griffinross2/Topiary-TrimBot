#include "block.h"

#include <format>

#include "imgui.h"

static int s_blockCount = 0;
static std::vector<BlockConnection> s_blockConnections;
static bool s_isConnecting = false;
static BlockConnection s_currentConnection = {"", "", 0, 0};
std::vector<std::shared_ptr<Block>> s_blocks;

Block::Block() : m_id(std::format("Block {}", s_blockCount)) {
    s_blockCount++;
}

Block::Block(std::string id) : m_id(id) {
    s_blockCount++;
}

void Block::setInput(int index, const std::variant<BLOCK_IO_TYPES>& value) {
    m_inputs.at(index).data = value;
    m_inputs.at(index).newData = true;
}

BlockIO& Block::getOutput(int index) {
    return m_outputs[index];
}

void Block::onUpdate() {}

void Block::drawInputs() {
    for (size_t i = 0; i < m_inputs.size(); ++i) {
        ImGui::SetCursorPos(ImVec2(0, ioSize / 2 + i * ioSize * 2));
        ImGui::PushID(std::format("{}_input_{}", m_id, i).c_str());
        if (ImGui::Button(std::format("", m_id, i).c_str(),
                          ImVec2(ioSize, ioSize))) {
            if (!s_isConnecting) {
                startBlockConnection(m_id, i, true);
            } else {
                endBlockConnection(m_id, i, true);
            }
        }
        ImGui::PopID();

        ImVec2 textSize = ImGui::CalcTextSize(m_inputs[i].name.c_str());
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(m_winX + ioSize + 10,
                   m_winY + ioSize + i * ioSize * 2 - textSize.y / 2),
            IM_COL32(255, 255, 255, 255), m_inputs[i].name.c_str());
    }
}

void Block::drawOutputs() {
    for (size_t i = 0; i < m_outputs.size(); ++i) {
        ImGui::SetCursorPos(
            ImVec2(blockWidth + ioSize, ioSize / 2 + i * ioSize * 2));
        ImGui::PushID(std::format("{}_output_{}", m_id, i).c_str());
        if (ImGui::Button(std::format("", m_id, i).c_str(),
                          ImVec2(ioSize, ioSize))) {
            if (!s_isConnecting) {
                startBlockConnection(m_id, i, false);
            } else {
                endBlockConnection(m_id, i, false);
            }
        }
        ImGui::PopID();

        ImVec2 textSize = ImGui::CalcTextSize(m_outputs[i].name.c_str());
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(m_winX + blockWidth + ioSize - 10 - textSize.x,
                   m_winY + ioSize + i * ioSize * 2 - textSize.y / 2),
            IM_COL32(255, 255, 255, 255), m_outputs[i].name.c_str());
    }
}

void Block::onRender() {
    ImGui::SetNextWindowSize(
        ImVec2(blockWidth + ioSize * 2,
               ioSize + (2 * std::max(m_inputs.size(), m_outputs.size()) - 1) *
                            ioSize),
        ImGuiCond_Always);

    ImGui::Begin(m_id.c_str(), nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    int winX = ImGui::GetWindowPos().x;
    int winY = ImGui::GetWindowPos().y;

    m_winX = winX;
    m_winY = winY;

    drawInputs();

    drawOutputs();

    ImGui::End();
}

std::shared_ptr<Block> getBlockById(const std::string& blockId) {
    for (const auto& block : s_blocks) {
        if (block->getId() == blockId) {
            return block;
        }
    }
    return nullptr;
}

void startBlockConnection(const std::string& blockId,
                          int startIndex,
                          bool input) {
    s_currentConnection.sourceId = input ? "" : blockId;
    s_currentConnection.sourceIndex = input ? 0 : startIndex;
    s_currentConnection.destId = input ? blockId : "";
    s_currentConnection.destIndex = input ? startIndex : 0;
    s_isConnecting = true;

    // If starting from an input, remove any existing connections to that input
    if (input) {
        s_blockConnections.erase(
            std::remove_if(s_blockConnections.begin(), s_blockConnections.end(),
                           [&](const BlockConnection& conn) {
                               return conn.destId == blockId &&
                                      conn.destIndex == startIndex;
                           }),
            s_blockConnections.end());
    }
}

void endBlockConnection(const std::string& blockId, int endIndex, bool input) {
    s_isConnecting = false;

    // inputs connected to each other
    if (input && s_currentConnection.sourceId.empty()) {
        s_currentConnection = {"", "", 0, 0};
        return;
    }
    // outputs connected to each other
    else if (!input && s_currentConnection.destId.empty()) {
        s_currentConnection = {"", "", 0, 0};
        return;
    }

    // block connecting to itself
    if (s_currentConnection.sourceId == blockId ||
        s_currentConnection.destId == blockId) {
        s_currentConnection = {"", "", 0, 0};
        return;
    }

    if (input) {
        s_currentConnection.destId = blockId;
        s_currentConnection.destIndex = endIndex;

        // Remove any existing connections to that input
        s_blockConnections.erase(
            std::remove_if(s_blockConnections.begin(), s_blockConnections.end(),
                           [&](const BlockConnection& conn) {
                               return conn.destId == blockId &&
                                      conn.destIndex == endIndex;
                           }),
            s_blockConnections.end());
    } else {
        s_currentConnection.sourceId = blockId;
        s_currentConnection.sourceIndex = endIndex;
    }

    s_blockConnections.push_back(s_currentConnection);
}

void updateBlocks() {
    for (const auto& block : s_blocks) {
        // Update the block then push its outputs to connected blocks
        block->onUpdate();
        for (size_t outIdx = 0; outIdx < block->getNumOutputs(); ++outIdx) {
            BlockIO& dataToSend = block->getOutput(static_cast<int>(outIdx));
            for (const auto& conn : s_blockConnections) {
                if (conn.sourceId == block->getId() &&
                    conn.sourceIndex == static_cast<int>(outIdx)) {
                    std::shared_ptr<Block> destBlock =
                        getBlockById(conn.destId);
                    int destIdx = conn.destIndex;
                    if (destBlock && dataToSend.newData) {
                        destBlock->setInput(destIdx, dataToSend.data);
                    }
                }
            }
            dataToSend.newData = false;
        }
    }
}

void renderBlocks() {
    for (const auto& block : s_blocks) {
        block->onRender();
    }
}

void renderBlockConnections() {
    for (const auto& conn : s_blockConnections) {
        std::shared_ptr<Block> sourceBlock = getBlockById(conn.sourceId);
        std::shared_ptr<Block> destBlock = getBlockById(conn.destId);
        if (sourceBlock && destBlock) {
            int x, y;
            sourceBlock->getPosition(x, y);
            ImVec2 sourcePos = ImVec2(x + blockWidth + 3 * ioSize / 2,
                                      y + ioSize + conn.sourceIndex * ioSize);

            destBlock->getPosition(x, y);
            ImVec2 destPos =
                ImVec2(x + ioSize / 2, y + ioSize + conn.destIndex * ioSize);

            ImGui::GetBackgroundDrawList()->AddLine(
                sourcePos, destPos, IM_COL32(0, 255, 0, 255), 2.0f);
        }
    }

    // In progess connection
    if (s_isConnecting) {
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        std::shared_ptr<Block> sourceBlock =
            getBlockById(s_currentConnection.sourceId);
        std::shared_ptr<Block> destBlock =
            getBlockById(s_currentConnection.destId);
        ImVec2 blockPos{0, 0};
        if (sourceBlock) {
            int x, y;
            sourceBlock->getPosition(x, y);
            blockPos =
                ImVec2(x + blockWidth + 3 * ioSize / 2,
                       y + ioSize + s_currentConnection.sourceIndex * ioSize);
        } else if (destBlock) {
            int x, y;
            destBlock->getPosition(x, y);
            blockPos =
                ImVec2(x + ioSize / 2,
                       y + ioSize + s_currentConnection.destIndex * ioSize);
        }
        ImGui::GetBackgroundDrawList()->AddLine(
            blockPos, mousePos, IM_COL32(255, 255, 255, 255), 2.0f);
    }

    // Also end connections if clicked outside of blocks
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && s_isConnecting) {
        s_isConnecting = false;
        s_currentConnection = {"", "", 0, 0};
    }
}