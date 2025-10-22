#include "int_input_block.h"

#include "imgui.h"

IntInputBlock::IntInputBlock() : Block() {
    // Initialize io
    m_outputs.push_back({"Value", int{0}});
}

IntInputBlock::IntInputBlock(std::string id) : Block(id) {
    // Initialize io
    m_outputs.push_back({"Value", int{0}});
}

void IntInputBlock::onRender() {
    Block::onRender();

    ImGui::Begin(m_id.c_str());

    ImGui::SetCursorPos(ImVec2(12.5, 12.5));
    ImGui::InputInt(std::format("##{}", m_id).c_str(),
                    &std::get<int>(m_outputs[0].data));

    ImGui::End();
}