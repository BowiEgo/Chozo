#include "TextureViewerPanel.hpp"

void TextureViewerPanel::Draw(const char* title) {
    if (!m_IsOpen) return;

    if (!ImGui::Begin(title, &m_IsOpen)) {
        ImGui::End();
        return;
    }

    ImGui::End();
}